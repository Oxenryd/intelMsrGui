//
// Created by oxenryd on 2026-02-03.
//

#include "CLI.h"
#include "IMT_ErrCode.h"
#include "MsrOps.h"
#include "OcMailbox.h"
#include "Parameters.hpp"

using namespace CLI;

int main(const int argc, char *argv[]) {

    const std::vector<std::string> args(argv, argv + argc);
    ParamPairs params;
    bool shouldLoop = false;
    auto EC = IMT_ErrCode::OK;
    PendingSave saveData{};
    IMT_CHECK_AND_RETURN_INT(EC, validateCliArgs(args, params));
    IMT_CHECK_AND_RETURN_INT(EC, execute(params, shouldLoop, saveData));
    // if (saveData.pending) {
    //     IMT_CHECK_AND_RETURN_INT(EC, MsrOps::saveCurrentSetting(saveData.path, saveData.name, saveData.presetIndex));
    // } else
    //     return print(shouldLoop);


    return 0;

}