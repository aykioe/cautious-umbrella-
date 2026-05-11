#pragma once

// Soil thresholds — trigger Telegram alert if outside range
#define SOIL_MOISTURE_MIN   30.0f   // % — below this: too dry
#define SOIL_MOISTURE_MAX   85.0f   // % — above this: overwatered
#define SOIL_TEMP_MIN       10.0f   // °C — below this: too cold
#define SOIL_TEMP_MAX       35.0f   // °C — above this: too hot
#define SOIL_PH_MIN          5.5f   // pH
#define SOIL_PH_MAX          7.5f   // pH
#define SOIL_EC_MAX          3.0f   // mS/cm — above this: salt buildup

// Spectral thresholds (NIR channel — indicator of plant stress)
#define NIR_LOW_WARNING     100     // Raw counts — low NIR may indicate poor canopy
