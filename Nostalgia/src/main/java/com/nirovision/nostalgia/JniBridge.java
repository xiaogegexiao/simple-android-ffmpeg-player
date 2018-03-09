package com.nirovision.nostalgia;

import android.graphics.Bitmap;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by xiaomei on 8/3/18.
 */

public class JniBridge {
    static {
        System.loadLibrary("nativeNiroFfmpeg");
    }

    private static JniBridge instance;

    private JniBridge () {}

    public static JniBridge getInstance() {
        if (instance == null) {
            instance = new JniBridge();
        }
        return instance;
    }
    public native int decodeStream(String streamUrl, VideoCallback callback);

    private void saveFrameToPath(Bitmap bitmap, String pPath) {
        int BUFFER_SIZE = 1024 * 8;
        try {
            File file = new File(pPath);
            file.createNewFile();
            FileOutputStream fos = new FileOutputStream(file);
            final BufferedOutputStream bos = new BufferedOutputStream(fos, BUFFER_SIZE);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, bos);
            bos.flush();
            bos.close();
            fos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public interface VideoCallback {
        void onVideo(int height, int width, byte[] bitmap);
    }
}
