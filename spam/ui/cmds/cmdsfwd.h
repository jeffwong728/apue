#ifndef SPAM_UI_CMDS_CMDS_FWD_H
#define SPAM_UI_CMDS_CMDS_FWD_H
#include <memory>

class SpamCmd;
class GeomCmd;

typedef std::shared_ptr<SpamCmd>       SPSpamCmd;
typedef std::shared_ptr<GeomCmd>       SPGeomCmd;
typedef std::vector<SPSpamCmd>         SPSpamCmdVector;

#endif //SPAM_UI_CMDS_CMDS_FWD_H