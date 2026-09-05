#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "model/data_model.h"

class UIManager {
public:
    UIManager();
    void begin();
    void updateData(const SystemMetrics& metrics);
    void nextPage();
    void prevPage();
    uint8_t getActivePage() const;
    void showOfflineScreen(const char* reason = "Unable to reach API");
    void switchPage(uint8_t pageIndex);
    TFT_eSPI& getTft() { return m_tft; }

private:
    TFT_eSPI m_tft;
    uint8_t m_activePage; // 0: Overview, 1: Storage, 2: Docker
    lv_obj_t* m_scrOverview;
    lv_obj_t* m_scrStorage;
    lv_obj_t* m_scrDocker;
    lv_obj_t* m_scrOffline;

    // Overview widgets
    lv_obj_t* m_lblHostName;
    lv_obj_t* m_lblCpuPercent;
    lv_obj_t* m_arcCpu;
    lv_obj_t* m_lblRamPercent;
    lv_obj_t* m_arcRam;
    lv_obj_t* m_lblTempCpu;
    lv_obj_t* m_lblNetRx;
    lv_obj_t* m_lblNetTx;
    lv_obj_t* m_lblUptime;

    // Storage widgets
    lv_obj_t* m_lblPoolPercent;
    lv_obj_t* m_arcPool;
    lv_obj_t* m_lblPoolUsedTotal;
    lv_obj_t* m_lblHdd1Status;
    lv_obj_t* m_lblHdd2Status;
    lv_obj_t* m_lblSpaceUsed;
    lv_obj_t* m_lblSpaceFree;
    lv_obj_t* m_lblSpaceTotal;

    // Docker widgets
    lv_obj_t* m_lblDockerTotal;
    lv_obj_t* m_lblDockerRunning;
    lv_obj_t* m_lblDockerStopped;
    lv_obj_t* m_listContainers;

    void buildOverviewScreen();
    void buildStorageScreen();
    void buildDockerScreen();
    void buildOfflineScreen();
};

#endif // UI_MANAGER_H
