//
// Created by yangw on 2018-4-1.
//

#ifndef WLMUSIC_PCMBEAN_H
#define WLMUSIC_PCMBEAN_H

#include <SoundTouch.h>

using namespace soundtouch;

class PCMBean {

public:
    char *buffer;
    int buffsize;

public:
    PCMBean(SAMPLETYPE *buffer, int size);
    ~PCMBean();


};


#endif //WLMUSIC_PCMBEAN_H
