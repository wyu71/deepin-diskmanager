#ifndef DISKMANAGERSERVICEPRIVATE_H
#define DISKMANAGERSERVICEPRIVATE_H

#include <QObject>
#include "diskoperation/partedcore.h"

namespace DiskManager {

class DiskManagerService;
class DiskManagerServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit DiskManagerServicePrivate(DiskManagerService *parent = nullptr);
    DeviceInfo getDeviceinfo();
    void getalldevice();
signals:

public slots:


private:
    DiskManager::PartedCore *m_partedcore;

public:
    DiskManagerService *q_ptr;
    Q_DECLARE_PUBLIC(DiskManagerService)
};
}
#endif // DISKMANAGERSERVICEPRIVATE_H
