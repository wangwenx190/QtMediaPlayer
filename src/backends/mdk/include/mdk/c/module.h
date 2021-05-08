/*
 * Copyright (c) 2020-2021 WangBin <wbsecg1 at gmail.com>
 * This file is part of MDK
 * MDK SDK: https://github.com/wang-bin/mdk-sdk
 * Free for opensource softwares or non-commercial use.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */
#pragma once
#ifdef __APPLE__
#import <Metal/Metal.h> /* to define swift bool type. but -fcxx-module error if include stdbool.h */
#endif
#include "MediaInfo.h"
#include "VideoFrame.h"
#include "RenderAPI.h"
#include "Player.h"
