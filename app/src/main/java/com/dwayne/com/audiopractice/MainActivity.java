package com.dwayne.com.audiopractice;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.dwayne.com.audioplayer.MuteEnum;
import com.dwayne.com.audioplayer.TimeInfoBean;
import com.dwayne.com.audioplayer.log.LogUtil;
import com.dwayne.com.audioplayer.player.AudioPlayer;
import com.dwayne.com.audioplayer.util.TimeUtil;

public class MainActivity extends AppCompatActivity {

    private AudioPlayer audioPlayer;
    private TextView tvTime, tvVolume;
    private SeekBar seekBarSeek, seekBarVolume;
    private int position = 0;
    private boolean isSeekBar = false;
    Handler handler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            if(msg.what == 1) {
                if(!isSeekBar) {
                    TimeInfoBean timeInfoBean = (TimeInfoBean) msg.obj;
                    tvTime.setText(String.format("%s/%s",
                            TimeUtil.secdsToDateFormat(timeInfoBean.getCurrentTime()),
                            TimeUtil.secdsToDateFormat(timeInfoBean.getTotalTime())));
                    seekBarSeek.setProgress(timeInfoBean.getCurrentTime() * 100 / timeInfoBean.getTotalTime());
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tvTime = findViewById(R.id.tv_time);
        tvVolume = findViewById(R.id.tv_volume);
        seekBarSeek = findViewById(R.id.seekbar_seek);
        seekBarVolume = findViewById(R.id.seekbar_volume);
        audioPlayer = new AudioPlayer();
        audioPlayer.setVolume(50);
        audioPlayer.setMute(MuteEnum.MUTE_CENTER);
        seekBarVolume.setProgress(audioPlayer.getVolumePercent());
        audioPlayer.setOnPreparedListener(() -> {
            LogUtil.d("open success");
            audioPlayer.start();
        });
        audioPlayer.setOnLoadListener(load -> LogUtil.d(load ? "loading" : "playing"));
        audioPlayer.setOnPauseResumeListener(pause -> LogUtil.d(pause ? "pause" :
                "resume"));
        audioPlayer.setOnTimeInfoListener(timeInfoBean -> {
            Message message = Message.obtain();
            message.what = 1;
            message.obj = timeInfoBean;
            handler.sendMessage(message);
        });
        audioPlayer.setOnErrorListener((code, msg) -> LogUtil.e(String.format("error " +
                "code:%d, msg:%s", code, msg)));
        audioPlayer.setOnCompleteListener(() -> LogUtil.d("play complete"));

        seekBarSeek.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                                          boolean fromUser) {
                if(audioPlayer.getDuration() > 0 && isSeekBar) {
                    position = audioPlayer.getDuration() * progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                audioPlayer.seek(position);
                isSeekBar = false;
            }
        });

        seekBarVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                                          boolean fromUser) {
                audioPlayer.setVolume(progress);
                tvVolume.setText(String.format("音量：%d", audioPlayer.getVolumePercent()));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    public void begin(View view) {
//        audioPlayer.setSource("/storage/emulated/0/Download/dcjlxk.mp3");
        audioPlayer.setSource("http://www.170mv.com/kw/antiserver.kuwo.cn/anti" +
                ".s?rid=MUSIC_90991360&response=res&format=mp3|aac&type=convert_url&br" +
                "=128kmp3&agent=iPhone&callback=getlink&jpcallback");
        audioPlayer.prepare();
    }

    public void pause(View view) {
        audioPlayer.pause();
    }

    public void resume(View view) {
        audioPlayer.resume();
    }

    public void stop(View view) {
        audioPlayer.stop();
    }

    public void seek(View view) {
        audioPlayer.seek(195);
    }

    public void next(View view) {
        audioPlayer.playNext("/storage/emulated/0/Download/dcjlxk.mp3");
    }

    public void left(View view) {
        audioPlayer.setMute(MuteEnum.MUTE_LEFT);
    }

    public void right(View view) {
        audioPlayer.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void center(View view) {
        audioPlayer.setMute(MuteEnum.MUTE_CENTER);
    }
}