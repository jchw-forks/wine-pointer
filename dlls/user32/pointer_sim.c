/*
 * Pointer simulation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "wingdi.h"
#include "controls.h"
#include "user_private.h"

#include "wine/unicode.h"
#include "wine/debug.h"
#include "winuser.h"

WINE_DEFAULT_DEBUG_CHANNEL(pointer);

#if defined(HAVE_LINUX_UINPUT_H)

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/uinput.h>

struct wine_synthetic_pointer
{
    CRITICAL_SECTION cs;
    POINTER_TYPE_INFO info;
    HANDLE thread;
    HANDLE wake;
    HANDLE destroyed;
    BOOL destroy;
    int fd;
};

/* Multiple of virtual desktop resolution to use for pointer devices. */
const static DWORD WINE_UINPUT_RESOLUTION_MULTIPLIER = 256;

BOOL wine_uinput_set_bit(int fd, int mask, int bit)
{
    if (ioctl(fd, mask, bit) < 0) {
        ERR("(%d %d %d): error in ioctl: %d %s", fd, mask, bit, errno, strerror(errno));
        return FALSE;
    }
    return TRUE;
}

BOOL wine_uinput_setup_axis(int fd, int axis, int min, int max)
{
    struct uinput_abs_setup abs;
    memset(&abs, 0, sizeof(abs));
    abs.code = axis;
    abs.absinfo.minimum = min;
    abs.absinfo.maximum = max;
    abs.absinfo.resolution = 1;
    if (ioctl(fd, UI_ABS_SETUP, &abs) < 0) {
        ERR("(%d %d %d %d): error in ioctl: %d %s", fd, axis, min, max, errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

BOOL wine_uinput_setup_device(int fd, int vendor, int product, const char *name)
{
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_VIRTUAL;
    usetup.id.vendor = vendor;
    usetup.id.product = product;
    lstrcpynA(usetup.name, name, UINPUT_MAX_NAME_SIZE);
    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        ERR("(%d %d %d %s): error in ioctl: %d %s", fd, vendor, product, name, errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

BOOL wine_uinput_create_device(int fd)
{
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        ERR("(%d): error in ioctl: %d %s", fd, errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

BOOL wine_uinput_emit(int fd, int type, int code, int val)
{
    struct input_event ie;
    memset(&ie, 0, sizeof(ie));
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    if (!write(fd, &ie, sizeof(ie))){
        ERR("(%d %d %d %d): error in write: %d %s", fd, type, code, val, errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return TRUE;
}

int pen_get_evdev_tool_for_info(const POINTER_PEN_INFO *info)
{
    if ((info->penFlags&PEN_FLAG_ERASER) != 0) {
        return BTN_TOOL_RUBBER;
    }
    return BTN_TOOL_PEN;
}

/*
 * Returns whether or not we should send data continously based on the current
 * frame of information. This is necessary due to the fact that libinput,
 * mutter, and other software will generate sporadic prox-out events if pen
 * tablet data stops flowing, to compensate for the fact that not all tablets
 * have reliable prox-out events.
 */
BOOL pen_should_send_data_continously(const POINTER_PEN_INFO *info)
{
    static const int active_mask = POINTER_FLAG_INRANGE|POINTER_FLAG_INCONTACT;

    return (info->pointerInfo.pointerFlags&active_mask) != 0 ? TRUE : FALSE;
}

DWORD WINAPI pen_thread(struct wine_synthetic_pointer *p)
{
    /* Interval to send events at when the pen is 'active'. */
    static const int active_interval_ms = 1;

    BOOL was_in_range, is_in_range, is_in_contact, destroy;
    int old_tool, current_tool, flags, x, y, last_x = -1, last_y = -1;

    POINTER_PEN_INFO current, last;

    memset(&last, 0, sizeof(POINTER_PEN_INFO));

    /* Wait for first frame indefinitely. */
    WaitForSingleObject(p->wake, INFINITE);

    while (1) {
        /* Grab current frame */
        EnterCriticalSection(&p->cs);
        destroy = p->destroy;
        current = p->info.penInfo;
        LeaveCriticalSection(&p->cs);

        if (destroy) {
            break;
        }

        was_in_range = (last.pointerInfo.pointerFlags&POINTER_FLAG_INRANGE) != 0;
        is_in_range = (current.pointerInfo.pointerFlags&POINTER_FLAG_INRANGE) != 0;
        is_in_contact = (current.pointerInfo.pointerFlags&POINTER_FLAG_INCONTACT) != 0;

        old_tool = pen_get_evdev_tool_for_info(&last);
        current_tool = pen_get_evdev_tool_for_info(&current);

        if (!wine_uinput_emit(p->fd, EV_KEY, BTN_TOUCH, is_in_contact)) {
            break;
        }

        x = current.pointerInfo.ptPixelLocation.x * WINE_UINPUT_RESOLUTION_MULTIPLIER;
        y = current.pointerInfo.ptPixelLocation.y * WINE_UINPUT_RESOLUTION_MULTIPLIER;

        if (!wine_uinput_emit(p->fd, EV_ABS, ABS_X, x)
         || !wine_uinput_emit(p->fd, EV_ABS, ABS_Y, y)) {
            break;
        }

        flags = current.pointerInfo.pointerFlags;

        if (!wine_uinput_emit(p->fd, EV_ABS, BTN_STYLUS, (flags&POINTER_FLAG_SECONDBUTTON) != 0)
         || !wine_uinput_emit(p->fd, EV_ABS, BTN_STYLUS2, (flags&POINTER_FLAG_THIRDBUTTON) != 0)
         || !wine_uinput_emit(p->fd, EV_ABS, BTN_STYLUS3, (flags&POINTER_FLAG_FOURTHBUTTON) != 0)) {
            break;
        }

        /* If we dropped out of range OR the tool changed -> set last tool to 0 */
        if ((was_in_range && !is_in_range) || (old_tool != current_tool)) {
            if (!wine_uinput_emit(p->fd, EV_KEY, old_tool, 0)) {
                break;
            }
        }

        /* If we entered into range OR the tool changed -> set current tool to 1 */
        if ((!was_in_range && is_in_range) || (old_tool != current_tool)) {
            if (!wine_uinput_emit(p->fd, EV_KEY, current_tool, 1)) {
                break;
            }
        }

        if ((current.penMask&PEN_MASK_PRESSURE) != 0) {
            if (!wine_uinput_emit(p->fd, EV_ABS, ABS_PRESSURE, current.pressure)) {
                break;
            }
        }

        if ((current.penMask&PEN_MASK_TILT_X) != 0) {
            if (!wine_uinput_emit(p->fd, EV_ABS, ABS_TILT_X, current.tiltX)) {
                break;
            }
        }

        if ((current.penMask&PEN_MASK_TILT_Y) != 0) {
            if (!wine_uinput_emit(p->fd, EV_ABS, ABS_TILT_Y, current.tiltY)) {
                break;
            }
        }

        if (!wine_uinput_emit(p->fd, EV_SYN, SYN_REPORT, 0)) {
            break;
        }

        if (pen_should_send_data_continously(&current)) {
            // Jitter the pressure continously while waiting for new input.
            while (WaitForSingleObject(p->wake, active_interval_ms) == WAIT_TIMEOUT) {
                wine_uinput_emit(p->fd, EV_ABS, ABS_PRESSURE, current.pressure > 0 ? current.pressure - 1 : 1);
                wine_uinput_emit(p->fd, EV_SYN, SYN_REPORT, 0);
                wine_uinput_emit(p->fd, EV_ABS, ABS_PRESSURE, current.pressure);
                wine_uinput_emit(p->fd, EV_SYN, SYN_REPORT, 0);
            }
        } else {
            WaitForSingleObject(p->wake, INFINITE);
        }

        last = current;
        last_x = x;
        last_y = y;
    }

    if (!destroy) {
        ERR("exiting pen_thread prematurely\n");
    }

    EnterCriticalSection(&p->cs);
    p->destroy = TRUE;
    LeaveCriticalSection(&p->cs);

    SetEvent(p->destroyed);

    return destroy == TRUE ? 0 : 1;
}

struct wine_synthetic_pointer *create_synthetic_pen(void)
{
    struct wine_synthetic_pointer *p;
    int fd, cx, cy;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if (fd < 0) {
        ERR("Failed to open /dev/uinput: %d %s\n", errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    wine_uinput_set_bit(fd, UI_SET_EVBIT, EV_KEY);
    wine_uinput_set_bit(fd, UI_SET_EVBIT, EV_ABS);
    wine_uinput_set_bit(fd, UI_SET_EVBIT, EV_SYN);

    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_TOOL_PEN);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_TOOL_RUBBER);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_TOUCH);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_0);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_1);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_2);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_3);
    wine_uinput_set_bit(fd, UI_SET_KEYBIT, BTN_4);

    wine_uinput_set_bit(fd, UI_SET_ABSBIT, ABS_X);
    wine_uinput_set_bit(fd, UI_SET_ABSBIT, ABS_Y);
    wine_uinput_set_bit(fd, UI_SET_ABSBIT, ABS_PRESSURE);
    wine_uinput_set_bit(fd, UI_SET_ABSBIT, ABS_TILT_X);
    wine_uinput_set_bit(fd, UI_SET_ABSBIT, ABS_TILT_Y);

    wine_uinput_set_bit(fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
    wine_uinput_set_bit(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);

    cx = GetSystemMetrics(SM_CXSCREEN) * WINE_UINPUT_RESOLUTION_MULTIPLIER;
    cy = GetSystemMetrics(SM_CYSCREEN) * WINE_UINPUT_RESOLUTION_MULTIPLIER;

    /* FIXME: min/max should be screen size even when using virtual desktop. */
    if (!wine_uinput_setup_axis(fd, ABS_X, 0, cx)
     || !wine_uinput_setup_axis(fd, ABS_Y, 0, cy)
     || !wine_uinput_setup_axis(fd, ABS_PRESSURE, 0, 1024)
     || !wine_uinput_setup_axis(fd, ABS_TILT_X, -90, 90)
     || !wine_uinput_setup_axis(fd, ABS_TILT_Y, -90, 90)) {
        return NULL;
    }

    if (!wine_uinput_setup_device(fd, 0x1209, 0x1993, "Wine Synthetic Pen")) {
        ERR("uinput_setup_device failed: %d %s\n", errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (!wine_uinput_create_device(fd)) {
        ERR("uinput_create_device failed: %d %s\n", errno, strerror(errno));
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                  sizeof(struct wine_synthetic_pointer));

    InitializeCriticalSection(&p->cs);

    p->fd = fd;
    p->wake = CreateEventW(NULL, FALSE, FALSE, NULL);
    p->destroyed = CreateEventW(NULL, FALSE, FALSE, NULL);
    p->thread = CreateThread(NULL, 0, pen_thread, p, 0, NULL);

    return p;
}

void destroy_synthetic_pointer(struct wine_synthetic_pointer *p)
{
    /* Signal pointer thread to exit (if it hasn't) */
    EnterCriticalSection(&p->cs);
    if (!p->destroy) {
        p->destroy = TRUE;
        SetEvent(p->wake);
    }
    LeaveCriticalSection(&p->cs);

    /* Wait for signal that thread is done */
    WaitForSingleObject(p->destroyed, INFINITE);

    /* Close event and thread handles */
    CloseHandle(p->thread);
    CloseHandle(p->destroyed);
    CloseHandle(p->wake);

    /* Close uinput fd */
    close(p->fd);

    HeapFree(GetProcessHeap(), 0, p);
}

BOOL inject_synthetic_event(struct wine_synthetic_pointer *p,
                            const POINTER_TYPE_INFO *info, UINT32 count)
{
    BOOL result;

    switch (info->type) {
    case PT_PEN:
        if (count != 1) {
            ERR("invalid number of cursors for PT_PEN: %d (must be 1)", count);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        EnterCriticalSection(&p->cs);
        if ((result = (p->thread != NULL))) {
            p->info = *info;
        }
        LeaveCriticalSection(&p->cs);
        SetEvent(p->wake);
        return result;

    case PT_TOUCH:
        FIXME("(%p %p %d): PT_TOUCH not supported yet\n", p, info, count);

        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;

    default:
        ERR("(%p %p %d): unknown type %d\n", p, info, count, info->type);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}

#else

struct wine_synthetic_pointer {};

wine_synthetic_pointer *create_synthetic_pen(void)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}

void destroy_synthetic_pointer(struct wine_synthetic_pointer *p)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
}

BOOL inject_synthetic_event(struct wine_synthetic_pointer *p,
                            const POINTER_TYPE_INFO *info, UINT32 count)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif

/**********************************************************************
 * CreateSyntheticPointerDevice (USER32.@)
 */

HSYNTHETICPOINTERDEVICE WINAPI CreateSyntheticPointerDevice(
    POINTER_INPUT_TYPE type,
    ULONG max,
    POINTER_FEEDBACK_MODE mode)
{
    switch (type) {
    case PT_PEN:
        if (max != 1) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }

        return (HSYNTHETICPOINTERDEVICE)create_synthetic_pen();

    case PT_TOUCH:
        FIXME("(%d %d %d): PT_TOUCH not supported yet\n", type, max, mode);

        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return NULL;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
}

/**********************************************************************
 * DestroySyntheticPointerDevice (USER32.@)
 */

void WINAPI DestroySyntheticPointerDevice( HSYNTHETICPOINTERDEVICE device )
{
    if (device == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return;
    }

    destroy_synthetic_pointer((struct wine_synthetic_pointer *)device);
}

/**********************************************************************
 * InjectSyntheticPointerInput (USER32.@)
 */

BOOL WINAPI InjectSyntheticPointerInput( HSYNTHETICPOINTERDEVICE device,
                                         const POINTER_TYPE_INFO *info,
                                         UINT32 count )
{
    if (device == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return inject_synthetic_event((struct wine_synthetic_pointer *)device, info, count);
}
