#pragma once

extern "C" {
#include "FreeRTOS.h"
}

#include <stdint.h>

static const uint32_t kSystemClockHz = 120000000U;
static const uint32_t kSensorTaskPeriodMs = 20U;
static const uint32_t kWirelessTaskPeriodMs = 50U;
static const uint32_t kSafetyTaskPeriodMs = 10U;
static const uint32_t kControlTaskPeriodMs = 25U;
static const uint32_t kDebugTaskPeriodMs = 250U;

static const uint16_t kMotorMaxSpeed = 1000U;
static const uint16_t kMotorSafeSpeed = 700U;
static const float kSensorTripThreshold = 80.0f;

static const uint16_t kDefaultTaskStackWords = 512U;
static const UBaseType_t kSensorTaskPriority = 2U;
static const UBaseType_t kWirelessTaskPriority = 2U;
static const UBaseType_t kSafetyTaskPriority = 3U;
static const UBaseType_t kControlTaskPriority = 2U;
static const UBaseType_t kDebugTaskPriority = 1U;
