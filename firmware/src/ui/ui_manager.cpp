#include "ui/ui_manager.h"
#include "ui/ui_theme.h"
#include "config.h"

static UIManager* s_uiManagerInstance = nullptr;

static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    if (s_uiManagerInstance) {
        s_uiManagerInstance->getTft().startWrite();
        s_uiManagerInstance->getTft().setAddrWindow(area->x1, area->y1, w, h);
        s_uiManagerInstance->getTft().pushColors((uint16_t *)&color_p->full, w * h, true);
        s_uiManagerInstance->getTft().endWrite();
    }
    lv_disp_flush_ready(disp);
}

UIManager::UIManager() : m_activePage(0) {
    s_uiManagerInstance = this;
}

void UIManager::begin() {
    // 1. Turn ON LCD Backlight Pin (GPIO 48)
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    // 2. Initialize ST7789 display controller
    m_tft.init();
    m_tft.setRotation(0);
    m_tft.invertDisplay(true);
    m_tft.fillScreen(TFT_BLACK);

    // 3. Initialize LVGL and register display driver
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[SCREEN_WIDTH * 20];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 20);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    buildOverviewScreen();
    buildStorageScreen();
    buildDockerScreen();
    buildOfflineScreen();
    switchPage(0);
}

void UIManager::buildOverviewScreen() {
    m_scrOverview = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrOverview, lv_color_black(), 0);
    lv_obj_set_style_pad_all(m_scrOverview, 8, 0);

    // Header
    lv_obj_t* header = lv_label_create(m_scrOverview);
    lv_label_set_text(header, "ZIMAOS");
    lv_obj_set_style_text_color(header, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(header, &lv_font_montserrat_14, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* subheader = lv_label_create(m_scrOverview);
    lv_label_set_text(subheader, "NAS SERVER");
    lv_obj_set_style_text_color(subheader, lv_color_make(150, 150, 150), 0);
    lv_obj_set_style_text_font(subheader, &lv_font_montserrat_14, 0);
    lv_obj_align(subheader, LV_ALIGN_TOP_MID, 0, 25);

    // Status indicator (● ONLINE)
    lv_obj_t* statusContainer = lv_obj_create(m_scrOverview);
    lv_obj_set_size(statusContainer, 60, 20);
    lv_obj_align(statusContainer, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(statusContainer, lv_color_make(0, 20, 0), 0);
    lv_obj_set_style_border_width(statusContainer, 1, 0);
    lv_obj_set_style_border_color(statusContainer, lv_color_make(0, 80, 0), 0);
    lv_obj_set_style_radius(statusContainer, 4, 0);
    lv_obj_set_style_pad_all(statusContainer, 2, 0);

    lv_obj_t* statusDot = lv_label_create(statusContainer);
    lv_label_set_text(statusDot, "●");
    lv_obj_set_style_text_color(statusDot, lv_color_make(0, 255, 0), 0); // Green
    lv_obj_set_style_text_font(statusDot, &lv_font_montserrat_14, 0);
    lv_obj_align(statusDot, LV_ALIGN_LEFT_MID, 2, 0);

    lv_obj_t* statusText = lv_label_create(statusContainer);
    lv_label_set_text(statusText, "ONLINE");
    lv_obj_set_style_text_color(statusText, lv_color_make(200, 200, 200), 0);
    lv_obj_set_style_text_font(statusText, &lv_font_montserrat_14, 0);
    lv_obj_align(statusText, LV_ALIGN_RIGHT_MID, -2, 0);

    // SYSTEM section
    lv_obj_t* sysTitle = lv_label_create(m_scrOverview);
    lv_label_set_text(sysTitle, "SYSTEM");
    lv_obj_set_style_text_color(sysTitle, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(sysTitle, &lv_font_montserrat_14, 0);
    lv_obj_align(sysTitle, LV_ALIGN_TOP_LEFT, 6, 75);

    lv_obj_t* cpuLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(cpuLabel, "CPU");
    lv_obj_set_style_text_color(cpuLabel, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(cpuLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(cpuLabel, LV_ALIGN_TOP_LEFT, 6, 95);

    m_lblCpuPercent = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblCpuPercent, "23%");
    lv_obj_set_style_text_color(m_lblCpuPercent, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(m_lblCpuPercent, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblCpuPercent, LV_ALIGN_TOP_RIGHT, -6, 95);

    lv_obj_t* ramLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(ramLabel, "RAM");
    lv_obj_set_style_text_color(ramLabel, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(ramLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(ramLabel, LV_ALIGN_TOP_LEFT, 6, 115);

    m_lblRamPercent = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblRamPercent, "41%");
    lv_obj_set_style_text_color(m_lblRamPercent, lv_color_make(180, 0, 255), 0); // Purple
    lv_obj_set_style_text_font(m_lblRamPercent, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblRamPercent, LV_ALIGN_TOP_RIGHT, -6, 115);

    lv_obj_t* tempLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(tempLabel, "TMP");
    lv_obj_set_style_text_color(tempLabel, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 6, 135);

    m_lblTempCpu = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblTempCpu, "46°C");
    lv_obj_set_style_text_color(m_lblTempCpu, lv_color_make(255, 255, 0), 0); // Yellow
    lv_obj_set_style_text_font(m_lblTempCpu, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblTempCpu, LV_ALIGN_TOP_RIGHT, -6, 135);

    // STORAGE section
    lv_obj_t* storageLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(storageLabel, "DISK");
    lv_obj_set_style_text_color(storageLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(storageLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(storageLabel, LV_ALIGN_TOP_LEFT, 6, 165);

    lv_obj_t* storageBg = lv_obj_create(m_scrOverview);
    lv_obj_set_size(storageBg, 120, 12);
    lv_obj_align(storageBg, LV_ALIGN_TOP_LEFT, 6, 185);
    lv_obj_set_style_bg_color(storageBg, lv_color_make(20, 20, 20), 0);
    lv_obj_set_style_border_width(storageBg, 1, 0);
    lv_obj_set_style_border_color(storageBg, lv_color_make(50, 50, 50), 0);
    lv_obj_set_style_radius(storageBg, 6, 0);

    m_storageBar = lv_obj_create(storageBg);
    lv_obj_set_size(m_storageBar, 0, 12);
    lv_obj_align(m_storageBar, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(m_storageBar, lv_color_make(0, 255, 0), 0); // Green
    lv_obj_set_style_radius(m_storageBar, 6, 0);

    lv_obj_t* storagePercent = lv_label_create(m_scrOverview);
    lv_label_set_text(storagePercent, "78%");
    lv_obj_set_style_text_color(storagePercent, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(storagePercent, &lv_font_montserrat_14, 0);
    lv_obj_align(storagePercent, LV_ALIGN_TOP_RIGHT, -6, 183);

    // NETWORK section
    lv_obj_t* networkLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(networkLabel, "NET");
    lv_obj_set_style_text_color(networkLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(networkLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(networkLabel, LV_ALIGN_TOP_LEFT, 6, 215);

    lv_obj_t* dlLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(dlLabel, "↓");
    lv_obj_set_style_text_color(dlLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(dlLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(dlLabel, LV_ALIGN_TOP_LEFT, 6, 235);

    m_lblNetRx = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblNetRx, "48.2 MB/s");
    lv_obj_set_style_text_color(m_lblNetRx, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(m_lblNetRx, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblNetRx, LV_ALIGN_TOP_LEFT, 22, 235);

    lv_obj_t* ulLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(ulLabel, "↑");
    lv_obj_set_style_text_color(ulLabel, lv_color_make(180, 0, 255), 0); // Purple
    lv_obj_set_style_text_font(ulLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(ulLabel, LV_ALIGN_TOP_LEFT, 6, 255);

    m_lblNetTx = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblNetTx, "12.4 MB/s");
    lv_obj_set_style_text_color(m_lblNetTx, lv_color_make(180, 0, 255), 0); // Purple
    lv_obj_set_style_text_font(m_lblNetTx, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblNetTx, LV_ALIGN_TOP_LEFT, 22, 255);

    // UPTIME section
    lv_obj_t* uptimeLabel = lv_label_create(m_scrOverview);
    lv_label_set_text(uptimeLabel, "UP");
    lv_obj_set_style_text_color(uptimeLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(uptimeLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(uptimeLabel, LV_ALIGN_TOP_LEFT, 6, 285);

    m_lblUptime = lv_label_create(m_scrOverview);
    lv_label_set_text(m_lblUptime, "12d 04h 32m");
    lv_obj_set_style_text_color(m_lblUptime, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(m_lblUptime, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblUptime, LV_ALIGN_TOP_RIGHT, -6, 285);
}

void UIManager::buildStorageScreen() {
    m_scrStorage = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrStorage, lv_color_black(), 0);
    lv_obj_set_style_pad_all(m_scrStorage, 6, 0);

    // Header
    lv_obj_t* header = lv_label_create(m_scrStorage);
    lv_label_set_text(header, "STORAGE");
    lv_obj_set_style_text_color(header, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(header, &lv_font_montserrat_14, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 6);

    // POOL section
    lv_obj_t* poolTitle = lv_label_create(m_scrStorage);
    lv_label_set_text(poolTitle, "POOL 1");
    lv_obj_set_style_text_color(poolTitle, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(poolTitle, &lv_font_montserrat_14, 0);
    lv_obj_align(poolTitle, LV_ALIGN_TOP_LEFT, 6, 22);

    m_lblPoolPercent = lv_label_create(m_scrStorage);
    lv_label_set_text(m_lblPoolPercent, "78%");
    lv_obj_set_style_text_color(m_lblPoolPercent, lv_color_make(0, 255, 0), 0); // Green
    lv_obj_set_style_text_font(m_lblPoolPercent, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblPoolPercent, LV_ALIGN_TOP_MID, 0, 40);

  m_lblPoolUsedTotal = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblPoolUsedTotal, "1.56 TB / 2.00 TB");
  lv_obj_set_style_text_color(m_lblPoolUsedTotal, lv_color_make(180, 180, 180), 0);
  lv_obj_set_style_text_font(m_lblPoolUsedTotal, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblPoolUsedTotal, LV_ALIGN_TOP_MID, 0, 74);

  // DISK STATUS section
  lv_obj_t* diskStatusTitle = lv_label_create(m_scrStorage);
  lv_label_set_text(diskStatusTitle, "DISK STATUS");
  lv_obj_set_style_text_color(diskStatusTitle, lv_color_make(0, 255, 255), 0); // Cyan
  lv_obj_set_style_text_font(diskStatusTitle, &lv_font_montserrat_14, 0);
  lv_obj_align(diskStatusTitle, LV_ALIGN_TOP_LEFT, 6, 98);

  // HDD 1
  m_lblHdd1Status = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd1Status, "HDD 1       ● OK");
  lv_obj_set_style_text_color(m_lblHdd1Status, lv_color_make(0, 255, 0), 0); // Green
  lv_obj_set_style_text_font(m_lblHdd1Status, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd1Status, LV_ALIGN_TOP_LEFT, 6, 118);

  m_lblHdd1Size = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd1Size, "2.00 TB");
  lv_obj_set_style_text_color(m_lblHdd1Size, lv_color_make(150, 150, 150), 0);
  lv_obj_set_style_text_font(m_lblHdd1Size, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd1Size, LV_ALIGN_TOP_RIGHT, -6, 118);

  m_lblHdd1Temp = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd1Temp, "42°C");
  lv_obj_set_style_text_color(m_lblHdd1Temp, lv_color_make(255, 255, 0), 0); // Yellow
  lv_obj_set_style_text_font(m_lblHdd1Temp, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd1Temp, LV_ALIGN_TOP_RIGHT, -6, 138);

  // HDD 2
  m_lblHdd2Status = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd2Status, "HDD 2       ● OK");
  lv_obj_set_style_text_color(m_lblHdd2Status, lv_color_make(0, 255, 0), 0); // Green
  lv_obj_set_style_text_font(m_lblHdd2Status, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd2Status, LV_ALIGN_TOP_LEFT, 6, 158);

  m_lblHdd2Size = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd2Size, "2.00 TB");
  lv_obj_set_style_text_color(m_lblHdd2Size, lv_color_make(150, 150, 150), 0);
  lv_obj_set_style_text_font(m_lblHdd2Size, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd2Size, LV_ALIGN_TOP_RIGHT, -6, 158);

  m_lblHdd2Temp = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblHdd2Temp, "44°C");
  lv_obj_set_style_text_color(m_lblHdd2Temp, lv_color_make(255, 255, 0), 0); // Yellow
  lv_obj_set_style_text_font(m_lblHdd2Temp, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblHdd2Temp, LV_ALIGN_TOP_RIGHT, -6, 178);

  // RAID section
  lv_obj_t* raidTitle = lv_label_create(m_scrStorage);
  lv_label_set_text(raidTitle, "RAID");
  lv_obj_set_style_text_color(raidTitle, lv_color_make(0, 255, 255), 0); // Cyan
  lv_obj_set_style_text_font(raidTitle, &lv_font_montserrat_14, 0);
  lv_obj_align(raidTitle, LV_ALIGN_TOP_LEFT, 6, 198);

  m_lblRaidHealth = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblRaidHealth, "✓ HEALTHY");
  lv_obj_set_style_text_color(m_lblRaidHealth, lv_color_make(0, 255, 0), 0); // Green
  lv_obj_set_style_text_font(m_lblRaidHealth, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblRaidHealth, LV_ALIGN_TOP_MID, 0, 218);

  m_lblRaidType = lv_label_create(m_scrStorage);
  lv_label_set_text(m_lblRaidType, "RAID 1");
  lv_obj_set_style_text_color(m_lblRaidType, lv_color_make(150, 150, 150), 0);
  lv_obj_set_style_text_font(m_lblRaidType, &lv_font_montserrat_14, 0);
  lv_obj_align(m_lblRaidType, LV_ALIGN_TOP_MID, 0, 238);
}

void UIManager::buildDockerScreen() {
    m_scrDocker = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrDocker, lv_color_black(), 0);
    lv_obj_set_style_pad_all(m_scrDocker, 6, 0);

    // Title
    lv_obj_t* header = lv_label_create(m_scrDocker);
    lv_label_set_text(header, "DOCKER");
    lv_obj_set_style_text_color(header, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(header, &lv_font_montserrat_14, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);

    // Counter bar
    lv_obj_t* counterBar = lv_obj_create(m_scrDocker);
    lv_obj_set_size(counterBar, 148, 20);
    lv_obj_align(counterBar, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_bg_color(counterBar, lv_color_make(15, 15, 15), 0);
    lv_obj_set_style_border_width(counterBar, 1, 0);
    lv_obj_set_style_border_color(counterBar, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_radius(counterBar, 2, 0);

    m_lblDockerTotal = lv_label_create(counterBar);
    lv_label_set_text(m_lblDockerTotal, "7 TOTAL");
    lv_obj_set_style_text_color(m_lblDockerTotal, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(m_lblDockerTotal, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblDockerTotal, LV_ALIGN_LEFT_MID, -36, 0);

    m_lblDockerRunning = lv_label_create(counterBar);
    lv_label_set_text(m_lblDockerRunning, "7 RUNNING");
    lv_obj_set_style_text_color(m_lblDockerRunning, lv_color_make(0, 255, 0), 0); // Green
    lv_obj_set_style_text_font(m_lblDockerRunning, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblDockerRunning, LV_ALIGN_CENTER, 0, 0);

    m_lblDockerStopped = lv_label_create(counterBar);
    lv_label_set_text(m_lblDockerStopped, "0 STOPPED");
    lv_obj_set_style_text_color(m_lblDockerStopped, lv_color_make(255, 0, 0), 0); // Red
    lv_obj_set_style_text_font(m_lblDockerStopped, &lv_font_montserrat_14, 0);
    lv_obj_align(m_lblDockerStopped, LV_ALIGN_RIGHT_MID, 36, 0);

    // Container list title
    lv_obj_t* containersTitle = lv_label_create(m_scrDocker);
    lv_label_set_text(containersTitle, "CONTAINERS");
    lv_obj_set_style_text_color(containersTitle, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(containersTitle, &lv_font_montserrat_14, 0);
    lv_obj_align(containersTitle, LV_ALIGN_TOP_LEFT, 6, 60);

    // Container list container
    m_listContainers = lv_obj_create(m_scrDocker);
    lv_obj_set_size(m_listContainers, 148, 180);
    lv_obj_align(m_listContainers, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(m_listContainers, lv_color_make(15, 15, 15), 0);
    lv_obj_set_style_border_width(m_listContainers, 1, 0);
    lv_obj_set_style_border_color(m_listContainers, lv_color_make(40, 40, 40), 0);
    lv_obj_set_style_radius(m_listContainers, 2, 0);
    lv_obj_set_style_pad_all(m_listContainers, 4, 0);

    // Create container items (we'll update these in updateData)
    m_containerItems.clear();
    for (int i = 0; i < 5; i++) { // Max 5 containers visible
        lv_obj_t* containerItem = lv_obj_create(m_listContainers);
        lv_obj_set_size(containerItem, 140, 22);
        lv_obj_align(containerItem, LV_ALIGN_TOP_MID, 0, i * 25);
        lv_obj_set_style_bg_color(containerItem, lv_color_make(20, 20, 20), 0);
        lv_obj_set_style_border_width(containerItem, 1, 0);
        lv_obj_set_style_border_color(containerItem, lv_color_make(30, 30, 30), 0);
        lv_obj_set_style_radius(containerItem, 2, 0);
        lv_obj_set_style_pad_all(containerItem, 4, 0);

        lv_obj_t* statusDot = lv_label_create(containerItem);
        lv_label_set_text(statusDot, "●");
        lv_obj_set_style_text_color(statusDot, lv_color_make(0, 255, 0), 0); // Green
        lv_obj_set_style_text_font(statusDot, &lv_font_montserrat_14, 0);
        lv_obj_align(statusDot, LV_ALIGN_LEFT_MID, 2, 0);

        lv_obj_t* containerName = lv_label_create(containerItem);
        lv_label_set_text(containerName, "Container Name");
        lv_obj_set_style_text_color(containerName, lv_color_make(180, 180, 180), 0);
        lv_obj_set_style_text_font(containerName, &lv_font_montserrat_14, 0);
        lv_obj_align(containerName, LV_ALIGN_LEFT_MID, 10, 0);

        lv_obj_t* containerStatus = lv_label_create(containerItem);
        lv_label_set_text(containerStatus, "UP");
        lv_obj_set_style_text_color(containerStatus, lv_color_make(0, 255, 0), 0); // Green
        lv_obj_set_style_text_font(containerStatus, &lv_font_montserrat_14, 0);
        lv_obj_align(containerStatus, LV_ALIGN_RIGHT_MID, -6, 0);

        m_containerItems.push_back({containerItem, statusDot, containerName, containerStatus});
    }
}

void UIManager::buildOfflineScreen() {
    m_scrOffline = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(m_scrOffline, lv_color_black(), 0);
    lv_obj_set_style_pad_all(m_scrOffline, 6, 0);

    lv_obj_t* title = lv_label_create(m_scrOffline);
    lv_label_set_text(title, "ZIMAOS");
    lv_obj_set_style_text_color(title, lv_color_make(0, 255, 255), 0); // Cyan
  lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  lv_obj_t* badge = lv_label_create(m_scrOffline);
  lv_label_set_text(badge, "● OFFLINE");
  lv_obj_set_style_text_color(badge, lv_color_make(255, 0, 0), 0); // Red
  lv_obj_set_style_text_font(badge, &lv_font_montserrat_14, 0);
  lv_obj_align(badge, LV_ALIGN_TOP_MID, 0, 38);

  lv_obj_t* msg = lv_label_create(m_scrOffline);
  lv_label_set_text(msg, "Unable to reach\nmonitor API\n\nRetrying...");
  lv_obj_set_style_text_color(msg, lv_color_make(150, 150, 150), 0);
  lv_obj_set_style_text_font(msg, &lv_font_montserrat_14, 0);
  lv_obj_align(msg, LV_ALIGN_CENTER, 0, 50);
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
    lv_refr_now(lv_disp_get_default());
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
    lv_refr_now(lv_disp_get_default());
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
    float poolPercent = metrics.storage.pools[0].usage_percent;
    snprintf(buf, sizeof(buf), "%.0f%%", poolPercent);
    lv_label_set_text(m_lblPoolPercent, buf);

    // Update storage bar width (max 120px)
    int barWidth = (int)((poolPercent / 100.0f) * 120.0f);
  lv_obj_set_size(m_storageBar, barWidth, 10);

  double usedTB = (double)metrics.storage.pools[0].used_bytes / 1e12;
  double totalTB = (double)metrics.storage.pools[0].total_bytes / 1e12;
  snprintf(buf, sizeof(buf), "%.2f TB / %.2f TB", usedTB, totalTB);
  lv_label_set_text(m_lblPoolUsedTotal, buf);
  }

  // Update disk status
  for (size_t i = 0; i < metrics.storage.disks.size() && i < 2; i++) {
    const auto& disk = metrics.storage.disks[i];
    if (i == 0) {
      snprintf(buf, sizeof(buf), "HDD 1       ● %s", 
               disk.health == "OK" ? "OK" : 
               disk.health == "warning" ? "WARN" : "ERR");
      lv_label_set_text(m_lblHdd1Status, buf);
      
      snprintf(buf, sizeof(buf), "%.2f TB", (double)disk.capacity_bytes / 1e12);
      lv_label_set_text(m_lblHdd1Size, buf);
      
      snprintf(buf, sizeof(buf), "%.0f°C", disk.temp_celsius);
      lv_label_set_text(m_lblHdd1Temp, buf);
    } else if (i == 1) {
      snprintf(buf, sizeof(buf), "HDD 2       ● %s", 
               disk.health == "OK" ? "OK" : 
               disk.health == "warning" ? "WARN" : "ERR");
      lv_label_set_text(m_lblHdd2Status, buf);
      
      snprintf(buf, sizeof(buf), "%.2f TB", (double)disk.capacity_bytes / 1e12);
      lv_label_set_text(m_lblHdd2Size, buf);
      
      snprintf(buf, sizeof(buf), "%.0f°C", disk.temp_celsius);
      lv_label_set_text(m_lblHdd2Temp, buf);
    }
  }

  // Update RAID status (simplified - use first pool status or default)
  if (!metrics.storage.pools.empty()) {
    const auto& pool = metrics.storage.pools[0];
    lv_label_set_text(m_lblRaidHealth, 
                     pool.usage_percent < 80 ? "✓ HEALTHY" : 
                     pool.usage_percent < 95 ? "⚠ WARNING" : "✗ CRITICAL");
    lv_label_set_text(m_lblRaidType, "RAID 1"); // Simplified - could get from metadata
  } else {
    lv_label_set_text(m_lblRaidHealth, "✓ HEALTHY");
    lv_label_set_text(m_lblRaidType, "RAID 1");
  }

  // Docker update
  snprintf(buf, sizeof(buf), "%d TOTAL", metrics.docker.total);
  lv_label_set_text(m_lblDockerTotal, buf);

  snprintf(buf, sizeof(buf), "%d RUNNING", metrics.docker.running);
  lv_label_set_text(m_lblDockerRunning, buf);

  snprintf(buf, sizeof(buf), "%d STOPPED", metrics.docker.stopped);
  lv_label_set_text(m_lblDockerStopped, buf);

  // Update container list
  size_t containerCount = std::min(metrics.docker.containers.size(), m_containerItems.size());
  for (size_t i = 0; i < containerCount; i++) {
    const auto& container = metrics.docker.containers[i];
    auto& item = m_containerItems[i];
    
    lv_label_set_text(item.containerName, container.name.c_str());
    
    // Status dot color
    lv_color_t dotColor = lv_color_make(0, 255, 0); // Green for running
    if (container.status == "exited" || container.status == "stopped") {
      dotColor = lv_color_make(255, 0, 0); // Red
    } else if (container.status == "paused") {
      dotColor = lv_color_make(255, 165, 0); // Orange
    } else if (container.status == "created") {
      dotColor = lv_color_make(0, 165, 255); // Blue
    }
    lv_obj_set_style_text_color(item.statusDot, dotColor, 0);
    
    lv_label_set_text(item.containerStatus, 
                     container.status == "running" ? "UP" : 
                     container.status == "exited" || container.status == "stopped" ? "DOWN" : 
                     container.status == "paused" ? "PAUSED" : 
                     container.status == "created" ? "CREATED" : container.status.c_str());
  }
  
  // Hide unused container items
  for (size_t i = containerCount; i < m_containerItems.size(); i++) {
    lv_obj_add_flag(m_containerItems[i].containerItem, LV_OBJ_FLAG_HIDDEN);
  }
  for (size_t i = 0; i < containerCount; i++) {
    lv_obj_clear_flag(m_containerItems[i].containerItem, LV_OBJ_FLAG_HIDDEN);
  }
}