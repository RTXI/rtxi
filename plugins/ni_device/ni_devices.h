/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef NI_DEVICES_H
#define NI_DEVICES_H

#include <osiTypes.h>
#include <default_gui_model.h>

struct ai_gain_t {
    char *name;
    u32 scale_idx;  // scale.h
    u32 gain_idx;   // ai.h
};

static ai_gain_t ni_mseries_ai_gain_default[] = {
    {
        "-10 to 10",
        0,
        1,
    },
    {
        "-5 to 5",
        1,
        2,
    },
    {
        "-2 to 2",
        2,
        3,
    },
    {
        "-1 to 1",
        3,
        4,
    },
    {
        "-.5 to .5",
        4,
        5,
    },
    {
        "-.2 to .2",
        5,
        6,
    },
    {
        "-.1 to .1",
        6,
        7,
    },
};
static size_t ni_mseries_ai_gain_default_size = sizeof(ni_mseries_ai_gain_default)/sizeof(ni_mseries_ai_gain_default[0]);

static ai_gain_t ni_mseries_ai_gain_622x[] = {
    {
        "-10 to 10",
        0,
        0,
    },
    {
        "-5 to 5",
        1,
        1,
    },
    {
        "-1 to 1",
        3,
        4,
    },
    {
        "-.2 to .2",
        5,
        5,
    },
};
static size_t ni_mseries_ai_gain_622x_size = sizeof(ni_mseries_ai_gain_622x)/sizeof(ni_mseries_ai_gain_622x[0]);

#define NI_MSERIES_622x_PERIOD_DIVISOR   80
#define NI_MSERIES_625x_PERIOD_DIVISOR   20
#define NI_MSERIES_628x_PERIOD_DIVISOR   40
#define NI_MSERIES_DELAY_DIVISOR          3

#define NI_MSERIES_ADC_RESET_DEFAULT      0
#define NI_MSERIES_ADC_RESET_625x         1

#define NI_MSERIES_SELECT_ACTIVE_DEFAULT  0
#define NI_MSERIES_SELECT_ACTIVE_622x     1

struct ni_device_t {
    size_t ai_count;
    size_t ao_count;
    size_t dio_count;
    ai_gain_t *ai_gains;
    size_t ai_gain_count;
    u32 period_divisor;
    u32 delay_divisor;
    bool adc_reset;
    bool select_active_high;
    unsigned short deviceID;
    char *name;
};

static struct ni_device_t ni_devices[] = {
    {
        16,
        0,
        24,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x70b0,
        "NI PCI-6220",
    },
    {
        16,
        2,
        24,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x70af,
        "NI PCI-6221",
    },
    {
        16,
        2,
        24,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x71bc,
        "NI PCI-6221 (37-pin)",
    },
    {
        32,
        0,
        48,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x70f2,
        "NI PCI-6224",
    },
    {
        80,
        2,
        24,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x716c,
        "NI PCI-6225",
    },
    {
        32,
        4,
        48,
        ni_mseries_ai_gain_622x,
        ni_mseries_ai_gain_622x_size,
        NI_MSERIES_622x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_622x,
        0x70aa,
        "NI PCI-6229",
    },
    {
        16,
        0,
        24,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70b4,
        "NI PCI-6250",
    },
    {
        16,
        2,
        24,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x717d,
        "NI PCIe-6251",
    },
    {
        16,
        2,
        24,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70b8,
        "NI PCI-6251",
    },
    {
        32,
        0,
        48,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70b7,
        "NI PCI-6254",
    },
    {
        32,
        4,
        48,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x717f,
        "NI PCIe-6259",
    },
    {
        32,
        4,
        48,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_625x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_625x,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70ab,
        "NI PCI-6259",
    },
    {
        16,
        0,
        24,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_628x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70b6,
        "NI PCI-6280",
    },
    {
        16,
        2,
        24,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_628x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70bd,
        "NI PCI-6281",
    },
    {
        32,
        0,
        48,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_628x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70bc,
        "NI PCI-6284",
    },
    {
        32,
        4,
        48,
        ni_mseries_ai_gain_default,
        ni_mseries_ai_gain_default_size,
        NI_MSERIES_628x_PERIOD_DIVISOR,
        NI_MSERIES_DELAY_DIVISOR,
        NI_MSERIES_ADC_RESET_DEFAULT,
        NI_MSERIES_SELECT_ACTIVE_DEFAULT,
        0x70ac,
        "NI PCI-6289",
    },
};
static size_t ni_devices_size = sizeof(ni_devices)/sizeof(ni_devices[0]);

#endif // NI_DEVICES_H
