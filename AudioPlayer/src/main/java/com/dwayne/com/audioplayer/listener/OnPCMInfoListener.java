package com.dwayne.com.audioplayer.listener;

/**
 * @author admin
 * @email dev1024@foxmail.com
 * @time 2020/12/2 11:15
 * @change
 * @chang time
 * @class describe
 */

public interface OnPCMInfoListener {
    void onPCMInfo(byte[] buffer, int bufferSize);

    void onPCMRate(int sampleRate, int bit, int channels);
}
