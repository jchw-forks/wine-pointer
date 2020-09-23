/*
 * Pointer API implementation
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
#include "wingdi.h"
#include "controls.h"
#include "user_private.h"

#include "wine/unicode.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(pointer);

/**********************************************************************
 * CreateSyntheticPointerDevice (USER32.@)
 */

HSYNTHETICPOINTERDEVICE WINAPI CreateSyntheticPointerDevice(
    POINTER_INPUT_TYPE type,
    ULONG max,
    POINTER_FEEDBACK_MODE mode)
{
    FIXME("(%d %d %d): stub\n", type, max, mode);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return NULL;
}

/**********************************************************************
 * DestroySyntheticPointerDevice (USER32.@)
 */

void WINAPI DestroySyntheticPointerDevice( HSYNTHETICPOINTERDEVICE device )
{
    FIXME("(%p): stub\n", device);
}

/***********************************************************************
 *		EnableMouseInPointer (USER32.@)
 */

BOOL WINAPI EnableMouseInPointer(BOOL enable)
{
    FIXME("(%#x) stub\n", enable);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * RegisterPointerDeviceNotifications (USER32.@)
 */

BOOL WINAPI RegisterPointerDeviceNotifications( HWND hwnd, BOOL notifyrange )
{
    FIXME("(%p %d): stub\n", hwnd, notifyrange);
    return TRUE;
}

/**********************************************************************
 * GetPointerCursorId (USER32.@)
 */

BOOL WINAPI GetPointerCursorId( UINT32 id, UINT32 *cursor_id )
{
    FIXME("(%d %p): stub\n", id, cursor_id);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerDevice (USER32.@)
 */

BOOL WINAPI GetPointerDevice( HANDLE device, POINTER_DEVICE_INFO *info )
{
    FIXME("(%p %p): stub\n", device, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerDeviceCursors (USER32.@)
 */

BOOL WINAPI GetPointerDeviceCursors( HANDLE device, UINT32 *count,
                                     POINTER_DEVICE_CURSOR_INFO *cursors )
{
    FIXME("(%p %p %p): stub\n", device, count, cursors);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerDeviceProperties (USER32.@)
 */

BOOL WINAPI GetPointerDeviceProperties( HANDLE device, UINT32 *count,
                                        POINTER_DEVICE_PROPERTY *props )
{
    FIXME("(%p %p %p): stub\n", device, count, props);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerDeviceRects (USER32.@)
 */

BOOL WINAPI GetPointerDeviceRects( HANDLE device, RECT *device_rect,
                                   RECT *display_rect )
{
    FIXME("(%p %p %p): stub\n", device, device_rect, display_rect);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerDevices (USER32.@)
 */

BOOL WINAPI GetPointerDevices( UINT32 *device_count,
                               POINTER_DEVICE_INFO *devices )
{
    FIXME("(%p %p): partial stub\n", device_count, devices);

    if (!device_count)
        return FALSE;

    if (devices)
        return FALSE;

    *device_count = 0;
    return TRUE;
}

/**********************************************************************
 * GetPointerFrameInfo (USER32.@)
 */

BOOL WINAPI GetPointerFrameInfo( UINT32 id, UINT32 *count,
                                 POINTER_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerFrameInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerFrameInfoHistory( UINT32 id, UINT32 *entry_count,
                                        UINT32 *pointer_count,
                                        POINTER_INFO *info )
{
    FIXME("(%d %p %p %p): stub\n", id, entry_count, pointer_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerFramePenInfo (USER32.@)
 */

BOOL WINAPI GetPointerFramePenInfo( UINT32 id, UINT32 *count,
                                    POINTER_PEN_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerFramePenInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerFramePenInfoHistory( UINT32 id, UINT32 *entry_count,
                                           UINT32 *pointer_count,
                                           POINTER_PEN_INFO *info )
{
    FIXME("(%d %p %p %p): stub\n", id, entry_count, pointer_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerFrameTouchInfo (USER32.@)
 */

BOOL WINAPI GetPointerFrameTouchInfo( UINT32 id, UINT32 *count,
                                      POINTER_TOUCH_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerFrameTouchInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerFrameTouchInfoHistory( UINT32 id, UINT32 *entry_count,
                                             UINT32 *pointer_count,
                                             POINTER_TOUCH_INFO *info )
{
    FIXME("(%d %p %p %p): stub\n", id, entry_count, pointer_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerInfo (USER32.@)
 */

BOOL WINAPI GetPointerInfo( UINT32 id, POINTER_INFO *info )
{
    FIXME("(%d %p): stub\n", id, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerInfoHistory( UINT32 id, UINT32 *entry_count,
                                   POINTER_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, entry_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerInputTransform (USER32.@)
 */

BOOL WINAPI GetPointerInputTransform( UINT32 id, UINT32 history_count,
                                      INPUT_TRANSFORM *transform )
{
    FIXME("(%d %d %p): stub\n", id, history_count, transform);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerPenInfo (USER32.@)
 */

BOOL WINAPI GetPointerPenInfo( UINT32 id, POINTER_PEN_INFO *info )
{
    FIXME("(%d %p): stub\n", id, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerPenInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerPenInfoHistory( UINT32 id, UINT32 *entry_count,
                                      POINTER_PEN_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, entry_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerTouchInfo (USER32.@)
 */

BOOL WINAPI GetPointerTouchInfo( UINT32 id, POINTER_TOUCH_INFO *info )
{
    FIXME("(%d %p): stub\n", id, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerTouchInfoHistory (USER32.@)
 */

BOOL WINAPI GetPointerTouchInfoHistory( UINT32 id, UINT32 *entry_count,
                                        POINTER_TOUCH_INFO *info )
{
    FIXME("(%d %p %p): stub\n", id, entry_count, info);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * GetPointerType (USER32.@)
 */

BOOL WINAPI GetPointerType(UINT32 id, POINTER_INPUT_TYPE *type)
{
    FIXME("(%d %p): stub\n", id, type);

    if(!id || !type)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *type = PT_MOUSE;
    return TRUE;
}

/**********************************************************************
 * GetRawPointerDeviceData (USER32.@)
 */

BOOL WINAPI GetRawPointerDeviceData( UINT32 id, UINT32 history_count,
                                     UINT32 prop_count,
                                     POINTER_DEVICE_PROPERTY *props,
                                     LONG *values )
{
    FIXME("(%d %d %d %p %p): stub\n", id, history_count, prop_count, props, values);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * InjectSyntheticPointerInput (USER32.@)
 */

BOOL WINAPI InjectSyntheticPointerInput( HSYNTHETICPOINTERDEVICE device,
                                         const POINTER_TYPE_INFO *info,
                                         UINT32 count )
{
    FIXME("(%p %p %d): stub\n", device, info, count);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * IsMouseInPointerEnabled (USER32.@)
 */

BOOL WINAPI IsMouseInPointerEnabled()
{
    return FALSE;
}

/**********************************************************************
 * RegisterPointerInputTarget (USER32.@)
 */

BOOL WINAPI RegisterPointerInputTarget( HWND hwnd, POINTER_INPUT_TYPE type )
{
    FIXME("(%p %d): stub\n", hwnd, type);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * SkipPointerFrameMessages (USER32.@)
 */

BOOL WINAPI SkipPointerFrameMessages( UINT32 id )
{
    FIXME("(%d): stub\n", id);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/**********************************************************************
 * UnregisterPointerInputTarget (USER32.@)
 */

BOOL WINAPI UnregisterPointerInputTarget( HWND hwnd, POINTER_INPUT_TYPE type )
{
    FIXME("(%p %d): stub\n", hwnd, type);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}