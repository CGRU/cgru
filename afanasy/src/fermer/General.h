#include "libafanasy/name_af.h"

#include <QStringList>
#include <time.h>
#include "Managers/Service/RadiolocationService.h"

namespace afermer
{
    class General : public QObject
    {
        Q_OBJECT
    public:
         General(QObject *parent = 0);
         Q_INVOKABLE bool serverExist() const;
         Q_INVOKABLE QString serverIP() const;
         Q_INVOKABLE QString userName() const;
         Q_INVOKABLE void app_exit() const;
    private:
        RadiolocationService::Ptr m_RLS;
    };
}
