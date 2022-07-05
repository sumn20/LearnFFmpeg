package com.project.learnffmpeg.videoplayer;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.widget.LinearLayout;

import com.project.learnffmpeg.R;
import com.project.learnffmpeg.databinding.ActivityMainBinding;
import com.project.learnffmpeg.databinding.ActivityVideoPlayerBinding;

public class VideoPlayerActivity extends AppCompatActivity {

    private VideoPlayer playerController;
    private ActivityVideoPlayerBinding binding;
    private SurfaceHolder surfaceHolder = null;
    private boolean isFirst = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player);
        binding = ActivityVideoPlayerBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        binding.glSurfaceView.getLayoutParams().height = getWindowManager().getDefaultDisplay().getWidth();
        SurfaceHolder mSurfaceHolder = binding.glSurfaceView.getHolder();
        mSurfaceHolder.addCallback(previewCallback);
        playerController = new VideoPlayer();
        binding.btnPlay.setOnClickListener(v -> {
            playerController.play();
        });
        binding.btnStop.setOnClickListener(v -> {
            playerController.pause();
        });

    }

    private SurfaceHolder.Callback previewCallback = new SurfaceHolder.Callback() {

        @Override
        public void surfaceCreated(@NonNull SurfaceHolder holder) {
            surfaceHolder = holder;
            if (isFirst) {
                playerController = new VideoPlayer() {
                    @Override
                    public void showLoadingDialog() {
                        super.showLoadingDialog();
                    }

                    @Override
                    public void hideLoadingDialog() {
                        super.hideLoadingDialog();

                    }

                    @Override
                    public void onCompletion() {
                        super.onCompletion();

                    }

                    @Override
                    public void videoDecodeException() {
                        super.videoDecodeException();
                    }

                    @Override
                    public void viewStreamMetaCallback(int width, int height, float duration) {
                        super.viewStreamMetaCallback(width, height, duration);
                        runOnUiThread(() -> {
                            int screenWidth = getWindowManager().getDefaultDisplay().getWidth();
                            int drawHeight = (int) ((float) screenWidth / ((float) width / (float) height));
                            ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)  binding.glSurfaceView.getLayoutParams();
                            params.height = drawHeight;
                            binding.glSurfaceView.setLayoutParams(params);
                            playerController.resetRenderSize(0, 0, screenWidth, drawHeight);
                        });

                    }

                };
                playerController.setUseMediaCodec(false);
                int width = getWindowManager().getDefaultDisplay().getWidth();
               // String path = "/storage/emulated/0/Download/test.mp4";
                String path = "http://1252463788.vod2.myqcloud.com/95576ef5vodtransgzp1252463788/e1ab85305285890781763144364/v.f20.mp4";
                playerController.init(path, holder.getSurface(), width, width, onInitialStatus -> {
                    // TODO: do your work here
                    Log.i("problem", "onInitialized called");
                });
                isFirst = false;
            } else {
                playerController.onSurfaceCreated(holder.getSurface());
            }


        }

        @Override
        public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
            playerController.resetRenderSize(0, 0, width, height);
        }

        @Override
        public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
            playerController.onSurfaceDestroyed(holder.getSurface());
        }
    };

    @Override
    protected void onDestroy() {
        super.onDestroy();
        playerController.stopPlay();
    }
}