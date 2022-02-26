//
// Created by zacke on 2/25/2022.
//

#ifndef I_SOUND_ENGINE_SOFAMODULE_H
#define I_SOUND_ENGINE_SOFAMODULE_H

#include "gtest/gtest.h"
#include "benchmark/benchmark.h"

#include "SOFA.h"
#include "SOFASimpleFreeFieldHRIR.h"

TEST(SOFA, isSofa)
{
    sofa::SimpleFreeFieldHRIR file(std::string("../HRIR/BKwHA_s.sofa"), netCDF::NcFile::read);

}

#endif //I_SOUND_ENGINE_SOFAMODULE_H
