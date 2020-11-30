package com.dwayne.com.audioplayer.player;

import android.text.TextUtils;

import com.dwayne.com.audioplayer.MuteEnum;
import com.dwayne.com.audioplayer.TimeInfoBean;
import com.dwayne.com.audioplayer.listener.OnCompleteListener;
import com.dwayne.com.audioplayer.listener.OnErrorListener;
import com.dwayne.com.audioplayer.listener.OnLoadListener;
import com.dwayne.com.audioplayer.listener.OnPauseResumeListener;
import com.dwayne.com.audioplayer.listener.OnPreparedListener;
import com.dwayne.com.audioplayer.listener.OnTimeInfoListener;
import com.dwayne.com.audioplayer.log.LogUtil;

/**
 * @author Dwayne
 * @email dev1024@foxmail.com
 * @time 20/11/23 14:12
 * @change
 * @chang time
 * @class describe
 */

public class AudioPlayer {

    static {
        System.loadLibrary("audioplayer-lib");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");
    }

    private static String source;
    private static boolean playNext = false;
    private static int duration = -1;
    private static int volumePercent = 100;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private OnPreparedListener onPreparedListener;
    private OnLoadListener onLoadListener;
    private OnPauseResumeListener onPauseResumeListener;
    private OnTimeInfoListener onTimeInfoListener;
    private OnErrorListener onErrorListener;
    private OnCompleteListener onCompleteListener;
    private static TimeInfoBean timeInfoBean;

    public AudioPlayer() {
    }

    public void setSource(String source) {
        this.source = source;
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void setOnLoadListener(OnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    public void setOnPauseResumeListener(OnPauseResumeListener onPauseResumeListener) {
        this.onPauseResumeListener = onPauseResumeListener;
    }

    public void setOnTimeInfoListener(OnTimeInfoListener onTimeInfoListener) {
        this.onTimeInfoListener = onTimeInfoListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnCompleteListener(OnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
    }

    public int getVolumePercent() {
        return volumePercent;
    }

    public void setMute(MuteEnum mute) {
        muteEnum = mute;
        n_mute(mute.getValue());
    }

    public void prepare() {
        if(TextUtils.isEmpty(source)) {
            LogUtil.d("source not be empty!");
            return;
        }
        new Thread(() -> n_prepare(source)).start();
    }

    public void start() {
        if(TextUtils.isEmpty(source)) {
            LogUtil.d("source not be empty!");
            return;
        }

        new Thread(() -> {
            setVolume(volumePercent);
            setMute(muteEnum);
            n_start();
        }).start();
    }

    public void pause() {
        n_pause();
        if(onPauseResumeListener != null) {
            onPauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        n_resume();
        if(onPauseResumeListener != null) {
            onPauseResumeListener.onPause(false);
        }
    }

    public void stop() {
        timeInfoBean = null;
        duration = -1;
        new Thread(() -> n_stop()).start();
    }

    public void seek(int secds) {
        n_seek(secds);
    }

    public void playNext(String url) {
        source = url;
        playNext = true;
        stop();
    }

    public int getDuration() {
        if(duration < 0) {
            duration = n_duration();
        }
        return duration;
    }

    /**
     * 给jni层调用
     */
    public void onCallPrepare() {
        if(onPreparedListener != null) {
            onPreparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if(onLoadListener != null){
            onLoadListener.onLoad(load);
        }
    }

    public void onTimeInfo(int currentTime, int totalTime) {
        if(onTimeInfoListener != null) {
            if(timeInfoBean == null) {
                timeInfoBean = new TimeInfoBean();
            }
            timeInfoBean.setCurrentTime(currentTime);
            timeInfoBean.setTotalTime(totalTime);
            onTimeInfoListener.onTimeInfo(timeInfoBean);
        }
    }

    public void setVolume(int percent) {
        if(percent >= 0 && percent <= 100) {
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public void onCallError(int code, String msg) {
        stop();
        if(onErrorListener != null) {
            onErrorListener.onError(code, msg);
        }
    }

    public void onCallComplete() {
        stop();
        if(onCompleteListener != null) {
            onCompleteListener.onComplete();
        }
    }

    public void onCallNext() {
        if(playNext) {
            playNext = false;
            prepare();
        }
    }

    private native void n_prepare(String  source);
    private native void n_start();
    private native void n_pause();
    private native void n_resume();
    private native void n_stop();
    private native void n_seek(int secds);
    private native int n_duration();
    private native void n_volume(int percent);
    private native void n_mute(int mute);

}
