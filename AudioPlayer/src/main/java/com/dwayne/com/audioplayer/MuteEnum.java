package com.dwayne.com.audioplayer;

/**
 * @author admin
 * @email dev1024@foxmail.com
 * @time 2020/11/30 15:28
 * @change
 * @chang time
 * @class describe
 */

public enum MuteEnum {

    MUTE_LEFT("LEFT", 0),
    MUTE_RIGHT("RIGHT", 1),
    MUTE_CENTER("CENTER", 2);

    private String name;
    private int value;

    MuteEnum(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
