#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "model/data_model.h"
#include <vector>

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
    lv_obj_t* m_lblCpuPercent;
    lv_obj_t* m_lblRamPercent;
    lv_obj_t* m_lblTempCpu;
    lv_obj_t* m_lblNetRx;
    lv_obj_t* m_lblNetTx;
    lv_obj_t* m_lblUptime;
    lv_obj_t* m_storageBar; // Storage progress bar fill

    // Storage widgets
    lv_obj_t* m_lblPoolPercent;
    lv_obj_t* m_lblPoolUsedTotal;
    lv_obj_t* m_lblHdd1Status;
    lv_obj_t* m_lblHdd1Size;
    lv_obj_t* m_lblHdd1Temp;
    lv_obj_t* m_lblHdd2Status;
    lv_obj_t* m_lblHdd2Size;
    lv_obj_t* m_lblHdd2Temp;
    lv_obj_t* m_lblRaidHealth;
    lv_obj_t* m_lblRaidType;

    // Docker widgets
    lv_obj_t* m_lblDockerTotal;
    lv_obj_t* m_lblDockerRunning;
    lv_obj_t* m_lblDockerStopped;
    lv_obj_t* m_listContainers;
    
    struct ContainerItem {
        lv_obj_t* containerItem;
        lv_obj_t* statusDot;
        lv_obj_t* containerName;
        lv_obj_t* containerStatus;
    };
    std::vector<ContainerItem> m_containerItems;

    void buildOverviewScreen();
    void buildStorageScreen();
    void buildDockerScreen();
    void buildOfflineScreen();
};

#endif // UI_MANAGER_H