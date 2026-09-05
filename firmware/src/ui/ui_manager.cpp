#include "ui/ui_manager.h"
#include "ui/ui_theme.h"
#include "config.h"

UIManager::UIManager() : m_activePage(0) {}

void UIManager::begin() {
    lv_init();
    buildOverviewScreen();
    buildStorageScreen();
    buildDockerScreen();
    buildOfflineScreen();
    switchPage(0);
}

void UIManager::buildOverviewScreen() {
    m_scrOverview = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrOverview, COLOR_BG, 0);

    // Title / Header
    lv_obj_t* title = lv_label_create(m_scrOverview);
    lv_label_set_text(title, "ZIMAOS");
    lv_obj_set_style_text_color(title, COLOR_CYAN, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* sub = lv_label_create(m_scrOverview);
    lv_label_set_text(sub, "NAS SERVER");
    lv_obj_set_style_text_color(sub, COLOR_TEXT_DIM, 0);
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 26);

    // ONLINE badge
    lv_obj_t* badge = lv_label_create(m_scrOverview);
    lv_label_set_text(badge, "ONLINE");
    lv_obj_set_style_text_color(badge, COLOR_GREEN, 0);
    lv_obj_align(badge, LV_ALIGN_TOP_MID, 0, 44);

    // SYSTEM Panel
    lv_obj_t* panelSys = lv_obj_create(m_scrOverview);
    lv_obj_set_size(panelSys, 160, 110);
    lv_obj_align(panelSys, LV_ALIGN_TOP_MID, 0, 65);
    lv_obj_set_style_bg_color(panelSys, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelSys, COLOR_CARD_BORDER, 0);

    lv_obj_t* lblSys = lv_label_create(panelSys);
    lv_label_set_text(lblSys, "SYSTEM");
    lv_obj_set_style_text_color(lblSys, COLOR_TEXT_DIM, 0);

    m_lblCpuPercent = lv_label_create(panelSys);
    lv_label_set_text(m_lblCpuPercent, "CPU  23%");
    lv_obj_set_style_text_color(m_lblCpuPercent, COLOR_CYAN, 0);
    lv_obj_align(m_lblCpuPercent, LV_ALIGN_TOP_LEFT, 0, 20);

    m_lblRamPercent = lv_label_create(panelSys);
    lv_label_set_text(m_lblRamPercent, "RAM  41%");
    lv_obj_set_style_text_color(m_lblRamPercent, COLOR_PURPLE, 0);
    lv_obj_align(m_lblRamPercent, LV_ALIGN_TOP_LEFT, 0, 40);

    m_lblTempCpu = lv_label_create(panelSys);
    lv_label_set_text(m_lblTempCpu, "TEMP 46 C");
    lv_obj_set_style_text_color(m_lblTempCpu, COLOR_YELLOW, 0);
    lv_obj_align(m_lblTempCpu, LV_ALIGN_TOP_LEFT, 0, 60);

    // NETWORK Panel
    lv_obj_t* panelNet = lv_obj_create(m_scrOverview);
    lv_obj_set_size(panelNet, 160, 75);
    lv_obj_align(panelNet, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_set_style_bg_color(panelNet, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelNet, COLOR_CARD_BORDER, 0);

    m_lblNetRx = lv_label_create(panelNet);
    lv_label_set_text(m_lblNetRx, "DL  48.2 MB/s");
    lv_obj_set_style_text_color(m_lblNetRx, COLOR_CYAN, 0);
    lv_obj_align(m_lblNetRx, LV_ALIGN_TOP_LEFT, 0, 15);

    m_lblNetTx = lv_label_create(panelNet);
    lv_label_set_text(m_lblNetTx, "UL  12.4 MB/s");
    lv_obj_set_style_text_color(m_lblNetTx, COLOR_PURPLE, 0);
    lv_obj_align(m_lblNetTx, LV_ALIGN_TOP_LEFT, 0, 35);

    // UPTIME Panel
    lv_obj_t* panelUp = lv_obj_create(m_scrOverview);
    lv_obj_set_size(panelUp, 160, 45);
    lv_obj_align(panelUp, LV_ALIGN_TOP_MID, 0, 260);
    lv_obj_set_style_bg_color(panelUp, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelUp, COLOR_CARD_BORDER, 0);

    m_lblUptime = lv_label_create(panelUp);
    lv_label_set_text(m_lblUptime, "12d 04h 32m");
    lv_obj_set_style_text_color(m_lblUptime, COLOR_CYAN, 0);
    lv_obj_align(m_lblUptime, LV_ALIGN_CENTER, 0, 0);
}

void UIManager::buildStorageScreen() {
    m_scrStorage = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrStorage, COLOR_BG, 0);

    // Header
    lv_obj_t* title = lv_label_create(m_scrStorage);
    lv_label_set_text(title, "STORAGE");
    lv_obj_set_style_text_color(title, COLOR_CYAN, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Pool Gauge Panel
    lv_obj_t* panelPool = lv_obj_create(m_scrStorage);
    lv_obj_set_size(panelPool, 160, 95);
    lv_obj_align(panelPool, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_bg_color(panelPool, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelPool, COLOR_CARD_BORDER, 0);

    m_lblPoolPercent = lv_label_create(panelPool);
    lv_label_set_text(m_lblPoolPercent, "78%");
    lv_obj_set_style_text_color(m_lblPoolPercent, COLOR_GREEN, 0);
    lv_obj_align(m_lblPoolPercent, LV_ALIGN_TOP_MID, 0, 10);

    m_lblPoolUsedTotal = lv_label_create(panelPool);
    lv_label_set_text(m_lblPoolUsedTotal, "1.56 TB / 2.00 TB");
    lv_obj_set_style_text_color(m_lblPoolUsedTotal, COLOR_TEXT_WHITE, 0);
    lv_obj_align(m_lblPoolUsedTotal, LV_ALIGN_TOP_MID, 0, 45);

    // Disk Status Panel
    lv_obj_t* panelDisk = lv_obj_create(m_scrStorage);
    lv_obj_set_size(panelDisk, 160, 90);
    lv_obj_align(panelDisk, LV_ALIGN_TOP_MID, 0, 135);
    lv_obj_set_style_bg_color(panelDisk, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelDisk, COLOR_CARD_BORDER, 0);

    m_lblHdd1Status = lv_label_create(panelDisk);
    lv_label_set_text(m_lblHdd1Status, "HDD 1  OK  42 C");
    lv_obj_set_style_text_color(m_lblHdd1Status, COLOR_GREEN, 0);
    lv_obj_align(m_lblHdd1Status, LV_ALIGN_TOP_LEFT, 0, 15);

    m_lblHdd2Status = lv_label_create(panelDisk);
    lv_label_set_text(m_lblHdd2Status, "HDD 2  OK  44 C");
    lv_obj_set_style_text_color(m_lblHdd2Status, COLOR_GREEN, 0);
    lv_obj_align(m_lblHdd2Status, LV_ALIGN_TOP_LEFT, 0, 40);

    // RAID Panel
    lv_obj_t* panelRaid = lv_obj_create(m_scrStorage);
    lv_obj_set_size(panelRaid, 160, 80);
    lv_obj_align(panelRaid, LV_ALIGN_TOP_MID, 0, 230);
    lv_obj_set_style_bg_color(panelRaid, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelRaid, COLOR_CARD_BORDER, 0);

    lv_obj_t* lblRaidHealth = lv_label_create(panelRaid);
    lv_label_set_text(lblRaidHealth, "HEALTHY");
    lv_obj_set_style_text_color(lblRaidHealth, COLOR_GREEN, 0);
    lv_obj_align(lblRaidHealth, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* lblRaidType = lv_label_create(panelRaid);
    lv_label_set_text(lblRaidType, "RAID 1 Mirroring");
    lv_obj_set_style_text_color(lblRaidType, COLOR_TEXT_DIM, 0);
    lv_obj_align(lblRaidType, LV_ALIGN_TOP_MID, 0, 35);
}

void UIManager::buildDockerScreen() {
    m_scrDocker = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrDocker, COLOR_BG, 0);

    // Title
    lv_obj_t* title = lv_label_create(m_scrDocker);
    lv_label_set_text(title, "DOCKER");
    lv_obj_set_style_text_color(title, COLOR_CYAN, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Counter Bar
    lv_obj_t* panelCounter = lv_obj_create(m_scrDocker);
    lv_obj_set_size(panelCounter, 160, 45);
    lv_obj_align(panelCounter, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_bg_color(panelCounter, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelCounter, COLOR_CARD_BORDER, 0);

    m_lblDockerTotal = lv_label_create(panelCounter);
    lv_label_set_text(m_lblDockerTotal, "7 TOTAL");
    lv_obj_set_style_text_color(m_lblDockerTotal, COLOR_CYAN, 0);
    lv_obj_align(m_lblDockerTotal, LV_ALIGN_LEFT_MID, 0, 0);

    m_lblDockerRunning = lv_label_create(panelCounter);
    lv_label_set_text(m_lblDockerRunning, "7 RUN");
    lv_obj_set_style_text_color(m_lblDockerRunning, COLOR_GREEN, 0);
    lv_obj_align(m_lblDockerRunning, LV_ALIGN_CENTER, 0, 0);

    m_lblDockerStopped = lv_label_create(panelCounter);
    lv_label_set_text(m_lblDockerStopped, "0 STOP");
    lv_obj_set_style_text_color(m_lblDockerStopped, COLOR_RED, 0);
    lv_obj_align(m_lblDockerStopped, LV_ALIGN_RIGHT_MID, 0, 0);

    // Container List Container
    m_listContainers = lv_obj_create(m_scrDocker);
    lv_obj_set_size(m_listContainers, 160, 175);
    lv_obj_align(m_listContainers, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_set_style_bg_color(m_listContainers, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(m_listContainers, COLOR_CARD_BORDER, 0);

    // System Alerts Panel
    lv_obj_t* panelAlerts = lv_obj_create(m_scrDocker);
    lv_obj_set_size(panelAlerts, 160, 45);
    lv_obj_align(panelAlerts, LV_ALIGN_TOP_MID, 0, 265);
    lv_obj_set_style_bg_color(panelAlerts, COLOR_CARD_BG, 0);
    lv_obj_set_style_border_color(panelAlerts, COLOR_CARD_BORDER, 0);

    lv_obj_t* lblAlert = lv_label_create(panelAlerts);
    lv_label_set_text(lblAlert, "NO ALERTS");
    lv_obj_set_style_text_color(lblAlert, COLOR_GREEN, 0);
    lv_obj_align(lblAlert, LV_ALIGN_CENTER, 0, 0);
}

void UIManager::buildOfflineScreen() {
    m_scrOffline = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrOffline, COLOR_BG, 0);

    lv_obj_t* title = lv_label_create(m_scrOffline);
    lv_label_set_text(title, "ZIMAOS");
    lv_obj_set_style_text_color(title, COLOR_CYAN, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t* badge = lv_label_create(m_scrOffline);
    lv_label_set_text(badge, "OFFLINE");
    lv_obj_set_style_text_color(badge, COLOR_RED, 0);
    lv_obj_align(badge, LV_ALIGN_TOP_MID, 0, 60);

    lv_obj_t* msg = lv_label_create(m_scrOffline);
    lv_label_set_text(msg, "Unable to reach\nmonitor API\n\nRetrying...");
    lv_obj_set_style_text_color(msg, COLOR_TEXT_DIM, 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 20);
}

void UIManager::switchPage(uint8_t pageIndex) {
    m_activePage = pageIndex % 3;
    if (m_activePage == 0) {
        lv_scr_load(m_scrOverview);
    } else if (m_activePage == 1) {
        lv_scr_load(m_scrStorage);
    } else {
        lv_scr_load(m_scrDocker);
    }
}

void UIManager::nextPage() {
    switchPage(m_activePage + 1);
}

void UIManager::prevPage() {
    switchPage((m_activePage + 2) % 3);
}

uint8_t UIManager::getActivePage() const {
    return m_activePage;
}

void UIManager::showOfflineScreen(const char* reason) {
    lv_scr_load(m_scrOffline);
}

void UIManager::updateData(const SystemMetrics& metrics) {
    if (!metrics.valid) {
        showOfflineScreen();
        return;
    }

    // Overview update
    char buf[64];
    snprintf(buf, sizeof(buf), "CPU  %.1f%%", metrics.cpu.usage_percent);
    lv_label_set_text(m_lblCpuPercent, buf);

    snprintf(buf, sizeof(buf), "RAM  %.1f%%", metrics.memory.usage_percent);
    lv_label_set_text(m_lblRamPercent, buf);

    snprintf(buf, sizeof(buf), "TEMP %.1f C", metrics.temperature.cpu_celsius);
    lv_label_set_text(m_lblTempCpu, buf);

    snprintf(buf, sizeof(buf), "DL  %.1f MB/s", metrics.network.total_rx_mb_s);
    lv_label_set_text(m_lblNetRx, buf);

    snprintf(buf, sizeof(buf), "UL  %.1f MB/s", metrics.network.total_tx_mb_s);
    lv_label_set_text(m_lblNetTx, buf);

    lv_label_set_text(m_lblUptime, metrics.uptime.formatted.c_str());

    // Storage update
    if (!metrics.storage.pools.empty()) {
        snprintf(buf, sizeof(buf), "%.1f%%", metrics.storage.pools[0].usage_percent);
        lv_label_set_text(m_lblPoolPercent, buf);

        double usedTB = (double)metrics.storage.pools[0].used_bytes / 1e12;
        double totalTB = (double)metrics.storage.pools[0].total_bytes / 1e12;
        snprintf(buf, sizeof(buf), "%.2f TB / %.2f TB", usedTB, totalTB);
        lv_label_set_text(m_lblPoolUsedTotal, buf);
    }

    if (metrics.storage.disks.size() >= 1) {
        snprintf(buf, sizeof(buf), "%s  OK  %.0f C", metrics.storage.disks[0].name.c_str(), metrics.storage.disks[0].temp_celsius);
        lv_label_set_text(m_lblHdd1Status, buf);
    }
    if (metrics.storage.disks.size() >= 2) {
        snprintf(buf, sizeof(buf), "%s  OK  %.0f C", metrics.storage.disks[1].name.c_str(), metrics.storage.disks[1].temp_celsius);
        lv_label_set_text(m_lblHdd2Status, buf);
    }

    // Docker update
    snprintf(buf, sizeof(buf), "%d TOT", metrics.docker.total);
    lv_label_set_text(m_lblDockerTotal, buf);

    snprintf(buf, sizeof(buf), "%d RUN", metrics.docker.running);
    lv_label_set_text(m_lblDockerRunning, buf);

    snprintf(buf, sizeof(buf), "%d STOP", metrics.docker.stopped);
    lv_label_set_text(m_lblDockerStopped, buf);
}
