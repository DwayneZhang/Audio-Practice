package com.dwayne.com.audioplayer.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;

import com.dwayne.com.audioplayer.MuteEnum;
import com.dwayne.com.audioplayer.TimeInfoBean;
import com.dwayne.com.audioplayer.listener.OnCompleteListener;
import com.dwayne.com.audioplayer.listener.OnErrorListener;
import com.dwayne.com.audioplayer.listener.OnLoadListener;
import com.dwayne.com.audioplayer.listener.OnPCMInfoListener;
import com.dwayne.com.audioplayer.listener.OnPauseResumeListener;
import com.dwayne.com.audioplayer.listener.OnPreparedListener;
import com.dwayne.com.audioplayer.listener.OnRecordTimeListener;
import com.dwayne.com.audioplayer.listener.OnTimeInfoListener;
import com.dwayne.com.audioplayer.listener.OnVolumeDBListener;
import com.dwayne.com.audioplayer.log.LogUtil;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author Dwayne
 * @email dev1024@foxmail.com
 * @time 20/11/23 14:12
 * @change
 * @chang time
 * @class describe
 */

public class AudioPlayer {

    private static String source;
    private static boolean playNext = false;
    private static int duration = -1;
    private static int volumePercent = 100;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private static float pitch = 1.0f;
    private static float speed = 1.0f;
    private static boolean initMediaCodec = false;
    private static TimeInfoBean timeInfoBean;

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

    private OnPreparedListener onPreparedListener;
    private OnLoadListener onLoadListener;
    private OnPauseResumeListener onPauseResumeListener;
    private OnTimeInfoListener onTimeInfoListener;
    private OnErrorListener onErrorListener;
    private OnCompleteListener onCompleteListener;
    private OnVolumeDBListener onVolumeDBListener;
    private OnRecordTimeListener onRecordTimeListener;
    private OnPCMInfoListener onPCMInfoListener;
    //mediacodec
    private MediaFormat encoderFormat = null;
    private MediaCodec encoder = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo bufferInfo = null;
    private int perPCMSize = 0;
    private byte[] outByteBuffer = null;
    private int aacSampleRate = 4;
    private double recordTime = 0;
    private int audioSampleRate = 0;

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

    public void setOnVolumeDBListener(OnVolumeDBListener onVolumeDBListener) {
        this.onVolumeDBListener = onVolumeDBListener;
    }

    public void setOnRecordTimeListener(OnRecordTimeListener onRecordTimeListener) {
        this.onRecordTimeListener = onRecordTimeListener;
    }

    public void setOnPCMInfoListener(OnPCMInfoListener onPCMInfoListener) {
        this.onPCMInfoListener = onPCMInfoListener;
    }

    public int getVolumePercent() {
        return volumePercent;
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
            setPitch(pitch);
            setSpeed(speed);
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
        stopRecord();
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

    public void setMute(MuteEnum mute) {
        muteEnum = mute;
        n_mute(mute.getValue());
    }

    public void setVolume(int percent) {
        if(percent >= 0 && percent <= 100) {
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public void setPitch(float p) {
        pitch = p;
        n_pitch(pitch);
    }

    public void setSpeed(float s) {
        speed = s;
        n_speed(speed);
    }

    public void startRecord(File outFile) {
        if(!initMediaCodec) {
            audioSampleRate = n_samplerate();
            initMediaCodec = true;
            initMediaCodec(audioSampleRate, outFile);
            n_record(true);
        }
    }

    public void stopRecord() {
        if(initMediaCodec) {
            n_record(false);
            releaseMedicacodec();
        }
    }

    public void pauseRecord() {
        if(initMediaCodec) {
            n_record(false);
        }
    }

    public void resumeRecord() {
        if(initMediaCodec) {
            n_record(true);
        }
    }

    public void cutAudioPlay(int start_time, int end_time, boolean showPCM) {
        if(n_cutaudio(start_time, end_time, showPCM)) {
            start();
        } else {
            stop();
        }
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
        if(onLoadListener != null) {
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

    public void onCallVolumeDB(int db) {
        if(onVolumeDBListener != null) {
            onVolumeDBListener.onDBValue(db);
        }
    }

    public void onCallPCMInfo(byte[] buffer, int bufferSize) {
        if(onPCMInfoListener != null) {
            onPCMInfoListener.onPCMInfo(buffer, bufferSize);
        }
    }

    public void onCallPCMRate(int sampleRate, int bit, int channels) {
        if(onPCMInfoListener != null) {
            onPCMInfoListener.onPCMRate(sampleRate, bit, channels);
        }
    }

    private native void n_prepare(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int secds);

    private native int n_duration();

    private native void n_volume(int percent);

    private native void n_mute(int mute);

    private native void n_pitch(float pitch);

    private native void n_speed(float speed);

    private native int n_samplerate();

    private native void n_record(boolean record);

    private native boolean n_cutaudio(int start_time, int end_time, boolean showPCM);

    private void initMediaCodec(int samplerate, File outfile) {

        try {
            aacSampleRate = getADTSsamplerate(samplerate);
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,
                    samplerate, 2);
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE,
                    MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            bufferInfo = new MediaCodec.BufferInfo();
            recordTime = 0;
            encoder.configure(encoderFormat, null, null,
                    MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outfile);
            encoder.start();
        } catch(IOException e) {
            e.printStackTrace();
        }
    }

    private void encodecPCMToAAC(int size, byte[] buffer) {
        if(buffer != null && encoder != null) {
            recordTime += size * 1.0 / (audioSampleRate * 2 * 2);
            if(onRecordTimeListener != null) {
                onRecordTimeListener.onRecord((int) recordTime);
            }
            int inputBufferIndex = encoder.dequeueInputBuffer(0);
            if(inputBufferIndex >= 0) {
                ByteBuffer byteBuffer = encoder.getInputBuffer(inputBufferIndex);
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferIndex, 0,
                        size, 0, 0);
            }
            int outputBufferIndex = encoder.dequeueOutputBuffer(bufferInfo, 0);
            while(outputBufferIndex >= 0) {
                perPCMSize = bufferInfo.size + 7;
                outByteBuffer = new byte[perPCMSize];

                ByteBuffer byteBuffer = encoder.getOutputBuffer(outputBufferIndex);
                byteBuffer.position(bufferInfo.offset);
                byteBuffer.limit(bufferInfo.offset + bufferInfo.size);

                addADtsHeader(outByteBuffer, perPCMSize, aacSampleRate);

                byteBuffer.get(outByteBuffer, 7, bufferInfo.size);
                byteBuffer.position(bufferInfo.offset);

                try {
                    outputStream.write(outByteBuffer, 0, perPCMSize);
                    encoder.releaseOutputBuffer(outputBufferIndex, false);
                    outputBufferIndex = encoder.dequeueOutputBuffer(bufferInfo, 0);
                    outByteBuffer = null;
                } catch(IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSsamplerate(int samplerate) {
        int rate = 4;
        switch(samplerate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseMedicacodec() {
        if(encoder == null) {
            return;
        }
        try {
            recordTime = 0;
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            bufferInfo = null;
            initMediaCodec = false;

        } catch(IOException e) {
            e.printStackTrace();
        } finally {
            if(outputStream != null) {
                try {
                    outputStream.close();
                } catch(IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }
}
