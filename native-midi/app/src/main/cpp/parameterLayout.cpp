//
// Created by Damien on 28/07/2022.
//

#include "parameterLayout.h"

void parameterLayout::add(std::string paramID, float initialValue) {
    parameterList.push_back(paramID);
    auto atom(std::unique_ptr<std::atomic<float>>(new std::atomic<float>(initialValue)));

    parameterMap.insert(std::make_pair(paramID, atom.get()));
    parameterAtomics.push_back(std::move(atom));
}

std::map<std::string, std::atomic < float> *>& parameterLayout::getParameterMap() {
    return parameterMap;
}
