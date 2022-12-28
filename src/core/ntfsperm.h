#ifndef DOOMSEEKER_CORE_NTFSPERM_H
#define DOOMSEEKER_CORE_NTFSPERM_H

// On NTFS file systems, ownership and permissions checking is disabled by
// default for performance reasons. The following int toggles it by
// incrementation and decrementation of its value.
// See: http://doc.qt.io/qt-5/qfileinfo.html#ntfs-permissions
#ifdef Q_OS_WIN32
#include <Qt>
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
// We'll need to declare an int with the same name to compile successfully in other platforms.
static int qt_ntfs_permission_lookup;
#endif


#endif
