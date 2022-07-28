//
// Created by Damien on 28/07/2022.
//

#ifndef NATIVE_MIDI_PARAMETERLAYOUT_H
#define NATIVE_MIDI_PARAMETERLAYOUT_H
#include <atomic>
#include <string>
#include <memory>
#include <map>
#include <list>


class parameterLayout {
public:
    void add(std::string paramID, float initialValue);
    std::map<std::string, std::atomic<float>*>& getParameterMap();

private:

    std::list<std::string> parameterList;
    std::list<std::unique_ptr<std::atomic<float>>> parameterAtomics;
    std::map<std::string, std::atomic<float>*> parameterMap;
};


#endif //NATIVE_MIDI_PARAMETERLAYOUT_H
