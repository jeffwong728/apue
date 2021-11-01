#ifndef SPAM_HELPER_COMMON_DEF_H
#define SPAM_HELPER_COMMON_DEF_H
#include <boost/core/noncopyable.hpp>
#include <string>

class CommonDef : private boost::noncopyable
{
public:
    CommonDef() = delete;

public:
    static const std::string &GetSpamDBNodeTypeAttrName() { return typeAttrName_s; }
    static const std::string &GetProjNodeTypeName() { return projNode_s; }
    static const std::string &GetProjPerspectiveAttrName() { return projPerspectiveAttrName_s; }
    static const std::string &GetStationTabAttrName() { return stationTabAttrName_s; }

    static const std::string &GetDBPathCfgPath() { return dbPathCfgPath_s; }
    static const std::string &GetModelImportDirPath() { return modelImportDirPath_s; }
    static const std::string &GetModelExportDirPath() { return modelExportDirPath_s; }
    static const std::string &GetProjCfgPath() { return projCfgPath_s; }
    static const std::string &GetProjPanelCfgPath() { return projPanelCfgPath_s; }
    static const std::string &GetImagePanelCfgPath() { return imagePanelCfgPath_s; }

private:
    static const std::string typeAttrName_s;
    static const std::string projNode_s;
    static const std::string dbPathCfgPath_s;
    static const std::string modelImportDirPath_s;
    static const std::string modelExportDirPath_s;
    static const std::string projCfgPath_s;
    static const std::string projPanelCfgPath_s;
    static const std::string imagePanelCfgPath_s;
    static const std::string projPerspectiveAttrName_s;
    static const std::string stationTabAttrName_s;
};

#endif  // SPAM_HELPER_COMMON_DEF_H