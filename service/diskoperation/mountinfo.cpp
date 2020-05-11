#include "mountinfo.h"
#include "utils.h"
#include "fsinfo.h"
#include <QStringList>
#include <limits.h>
#include <mntent.h>
#include <QFile>

namespace DiskManager {
static MountInfo::MountMapping mount_info;
static MountInfo::MountMapping fstab_info;

void MountInfo::load_cache()
{
    mount_info.clear();
    fstab_info.clear();

    read_mountpoints_from_file("/proc/mounts", mount_info);
    read_mountpoints_from_file_swaps("/proc/swaps", mount_info);

    if (! have_rootfs_dev(mount_info))
        // Old distributions only contain 'rootfs' and '/dev/root' device names
        // for the / (root) file system in /proc/mounts with '/dev/root' being a
        // block device rather than a symlink to the true device.  This prevents
        // identification, and therefore busy detection, of the device containing
        // the / (root) file system.  Used to read /etc/mtab to get the root file
        // system device name, but this contains an out of date device name after
        // the mounting device has been dynamically removed from a multi-device
        // btrfs, thus identifying the wrong device as busy.  Instead fall back
        // to reading mounted file systems from the output of the mount command,
        // but only when required.
        read_mountpoints_from_mount_command(mount_info);

    read_mountpoints_from_file("/etc/fstab", fstab_info);

    // Sort the mount points and remove duplicates ... (no need to do this for fstab_info)
    MountMapping::iterator iter_mp;
    for (iter_mp = mount_info.begin() ; iter_mp != mount_info.end() ; ++ iter_mp) {
        std::sort(iter_mp.value().mountpoints.begin(), iter_mp.value().mountpoints.end());

        iter_mp.value().mountpoints.erase(
            std::unique(iter_mp.value().mountpoints.begin(), iter_mp.value().mountpoints.end()),
            iter_mp.value().mountpoints.end());
    }
}

bool MountInfo::is_dev_mounted(const QString &path)
{
    return is_dev_mounted(BlockSpecial(path));
}

bool MountInfo::is_dev_mounted(const BlockSpecial &bs)
{
    MountMapping::const_iterator iter_mp = mount_info.find(bs);
    return iter_mp != mount_info.end();
}

void MountInfo::read_mountpoints_from_file(const QString &filename, MountInfo::MountMapping &map)
{
    FILE *fp = setmntent(filename.toStdString().c_str(), "r");
    if (fp == NULL)
        return;

    struct mntent *p = NULL;
    while ((p = getmntent(fp)) != NULL) {
        QString node = p->mnt_fsname;
        QString mountpoint = p->mnt_dir;

        QString uuid = Utils::regexp_label(node, "^UUID=(.*)");
        if (! uuid.isEmpty())
            node = FsInfo::get_path_by_uuid(uuid);

        QString label = Utils::regexp_label(node, "^LABEL=(.*)");
        if (! label.isEmpty())
            node = FsInfo::get_path_by_label(label);

        if (! node.isEmpty())
            add_mountpoint_entry(map, node, mountpoint, parse_readonly_flag(p->mnt_opts));
    }

    endmntent(fp);
}

void MountInfo::add_mountpoint_entry(MountInfo::MountMapping &map, QString &node, QString &mountpoint, bool readonly)
{
    // Only add node path if mount point exists
    QFile file(mountpoint);
    if (file.exists()) {
        // Map::operator[] default constructs MountEntry for new keys (nodes).
        MountEntry &mountentry = map[BlockSpecial(node)];
        mountentry.readonly = mountentry.readonly || readonly;
        mountentry.mountpoints.push_back(mountpoint);
    }
}

bool MountInfo::parse_readonly_flag(const QString &str)
{

}

void MountInfo::read_mountpoints_from_file_swaps(const QString &filename, MountInfo::MountMapping &map)
{

}

bool MountInfo::have_rootfs_dev(MountInfo::MountMapping &map)
{

}

void MountInfo::read_mountpoints_from_mount_command(MountInfo::MountMapping &map)
{
    QString output;
    QString error;
    if (! Utils::executcmd("mount", output, error)) {
        QStringList lines;
        lines = output.split("\n");
        for (unsigned int i = 0 ; i < lines .size() ; i ++) {
            // Process line like "/dev/sda3 on / type ext4 (rw)"
            QString node = Utils::regexp_label(lines[ i ], "^([^[:blank:]]+) on ");
            QString mountpoint = Utils::regexp_label(lines[ i ], "^[^[:blank:]]+ on ([^[:blank:]]+) ");
            QString mntopts = Utils::regexp_label(lines[i], " type [^[:blank:]]+ \\(([^\\)]*)\\)");
            if (! node.isEmpty())
                add_mountpoint_entry(map, node, mountpoint, parse_readonly_flag(mntopts));
        }
    }
}

const MountEntry &MountInfo::find(const MountInfo::MountMapping &map, const QString &path)
{

}

}
