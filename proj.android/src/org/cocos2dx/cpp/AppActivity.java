/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2014 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 ****************************************************************************/
package org.cocos2dx.cpp;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.cocos2dx.lib.Cocos2dxActivity;
import org.cocos2dx.lib.Cocos2dxGLSurfaceView;
import org.cocos2dx.lib.Cocos2dxHelper;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.nifty.cloud.mb.FindCallback;
import com.nifty.cloud.mb.NCMB;
import com.nifty.cloud.mb.NCMBException;
import com.nifty.cloud.mb.NCMBObject;
import com.nifty.cloud.mb.NCMBQuery;
import com.nifty.cloud.mb.SaveCallback;

public class AppActivity extends Cocos2dxActivity {
    private static AdView         adView;
    private static InterstitialAd interstitial;
    private static final String   TAG        = "AppActivity";
    private static final String   APP_KEY    = "20260d65d952801a5d44619a0a001f6f2901e42a216c31a9030a436f7ab65e42";
    private static final String   CLIENT_KEY = "358d2a0959ab857d8c85c0696ad8fc18aa58f18eb4305924436b130b16ca7035";
    private final int             lp         = LinearLayout.LayoutParams.WRAP_CONTENT;
    private static final String   KEY_USERID = "key_userid";
    private static final String   AD_UNIT_ID = "ca-app-pub-7207474321253495/6808608760";

    private static final String   ASC        = "asc";
    private static final String   DESC       = "desc";

    public Cocos2dxGLSurfaceView  mGLView;

    @SuppressLint("NewApi")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        NCMB.initialize(this, APP_KEY, CLIENT_KEY);

        // ユーザIDが登録されていない場合はユーザIDを取得する
        if (loadUserId(getContext()) == null) {
            createUser();
        }

    }

    protected void onStart() {
        super.onStart();

        createInterstitial();
        loadInterstitial();
    }

    public void createInterstitial() {
        interstitial = new InterstitialAd(this);
        interstitial.setAdUnitId(AD_UNIT_ID);
        interstitial.setAdListener(new AdListener() {
            @Override
            public void onAdLoaded() {
                Log.d(TAG, "広告が読み込まれました。");
            }

            public void onAdFailedToLoad(int errorCode) {
                switch (errorCode) {
                    case AdRequest.ERROR_CODE_INTERNAL_ERROR:
                        Log.d(TAG, "Internal error");
                        break;
                    case AdRequest.ERROR_CODE_INVALID_REQUEST:
                        Log.d(TAG, "Invalid request");
                        break;
                    case AdRequest.ERROR_CODE_NETWORK_ERROR:
                        Log.d(TAG, "Network Error");
                        break;
                    case AdRequest.ERROR_CODE_NO_FILL:
                        Log.d(TAG, "No fill");
                        break;
                }
            }

            public void onAdLeftApplication() {
                // Toast.makeText(AppActivity.this, "他のアプリが開かれました。",
                // Toast.LENGTH_SHORT).show();
            }

            public void onAdOpened() {
                // Toast.makeText(AppActivity.this, "広告が開かれました。",
                // Toast.LENGTH_SHORT).show();
            }

            public void onAdClosed() {
                // Toast.makeText(AppActivity.this, "広告が閉じられました。",
                // Toast.LENGTH_SHORT).show();
            }
        });
    }

    public void loadInterstitial() {
        AdRequest adRequest = new AdRequest.Builder()
                .addTestDevice(AdRequest.DEVICE_ID_EMULATOR)
                .addTestDevice("02F68F5ED171F12FA2207211A8F052F5")
                .addTestDevice("B3D38DAD43CAFD59B71A06F8153CC143").build();
        interstitial.loadAd(adRequest);
    }

    public static void displayInterstitial() {
        if (interstitial.isLoaded()) {
            interstitial.show();
            Log.d(TAG, "広告を表示しました。");
        } else {
            Log.d(TAG, "広告がまだ読み込まれていません。");
        }
    }

    public static void executeJava() {
        Log.d(TAG, "executeJava()実行！★");

        Cocos2dxHelper.getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    Thread.sleep(1 * 1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                displayInterstitial();
            }
        });
    }

    public static void executeRegistScore(int score) {
        Log.d(TAG, "executeRegistScore");

        final String SCORE = "score";
        final String COL_USER_ID = "userId";
        final String COL_SCORE = "score";
        final String userId = loadUserId(getContext());

        NCMBObject tScore = new NCMBObject(SCORE);
        tScore.put(COL_USER_ID, userId);
        tScore.put(COL_SCORE, score);
        tScore.saveInBackground();
    }

    public static void executeRegistTime(float time) {
        Log.d(TAG, "executeRegistTime");

        final String TIME = "time";
        final String COL_USER_ID = "userId";
        final String COL_TIME = "time";
        final String userId = loadUserId(getContext());

        NCMBObject tTime = new NCMBObject(TIME);
        tTime.put(COL_USER_ID, userId);
        tTime.put(COL_TIME, time);
        tTime.saveInBackground();
    }

    // NCMB連携
    private static void createUser() {
        final String USER = "User";
        final String COL_TIME_STAMP = "timestamp";
        final Long timestamp = System.currentTimeMillis();

        // Userテーブル登録
        NCMBObject user = new NCMBObject(USER);
        user.put(COL_TIME_STAMP, timestamp);
        user.saveInBackground(new SaveCallback() {

            @Override
            public void done(NCMBException e) {
                // 登録時のタイムスタンプをキーにユーザID取得し、端末に保存
                NCMBQuery<NCMBObject> query = NCMBQuery.getQuery(USER);
                query.whereEqualTo(COL_TIME_STAMP, timestamp);
                query.findInBackground(new FindCallback<NCMBObject>() {
                    @Override
                    public void done(List<NCMBObject> result, NCMBException e) {
                        if (result != null && !result.isEmpty()) {
                            saveUserId(getContext(), result.get(0)
                                    .getObjectId());

                        } else {
                            // TODO
                            // エラー時処理
                            AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                                    getContext());

                            alertDialogBuilder.setTitle("あわわわわ・・・");
                            alertDialogBuilder
                                    .setMessage("アプリの起動中に通信エラーが発生しちゃいました。\n"
                                            + "データ通信設定をご確認のうえ、\n"
                                            + "再度アプリを起動しなおしていただけませんでしょうか。");
                            alertDialogBuilder.show();
                        }
                    }
                });
            }
        });
    }

    public static String executeFindTime() {
        final String TABLE_NAME = "time";
        final String COL_USERID = "userId";
        final String COL_SCORE = "time";
        String userId = loadUserId(getContext());

        return executeFindTable(TABLE_NAME, COL_USERID, userId, COL_SCORE, ASC,
                "time");
    }

    public static String executeFindScore() {
        final String TABLE_NAME = "score";
        final String COL_USERID = "userId";
        final String COL_SCORE = "score";
        String userId = loadUserId(getContext());

        return executeFindTable(TABLE_NAME, COL_USERID, userId, COL_SCORE,
                DESC, "");
    }

    public static String executeFindTable(String tableName,
            String whereColName, String whereVal, String orderByColName,
            String orderByVal, String format) {
        Log.d(TAG, "executeFindTable");

        final String TABLE_NAME = tableName;
        final String WHERE_COL_NAME = whereColName;
        final String ORDER_BY_COL_NAME = orderByColName;

        List<NCMBObject> scoreList = new ArrayList<NCMBObject>();
        String ret = "";

        NCMBQuery<NCMBObject> query = NCMBQuery.getQuery(TABLE_NAME);
        query.whereEqualTo(WHERE_COL_NAME, whereVal);

        if (ASC.equals(orderByVal)) {
            query.orderByAscending(ORDER_BY_COL_NAME);
        } else if (DESC.equals(orderByVal)) {
            query.orderByDescending(ORDER_BY_COL_NAME);
        }

        try {
            scoreList = query.find();
        } catch (NCMBException e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }

        int i = 0;

        for (Iterator<NCMBObject> it = scoreList.iterator(); it.hasNext();) {
            NCMBObject record = it.next();

            String val;

            if (format == "time") {
                val = String
                        .format("%.2f", record.getDouble(ORDER_BY_COL_NAME));
            } else {
                val = record.get(ORDER_BY_COL_NAME).toString();
            }

            Log.d(TAG, "value[" + i + "]: " + val);
            i++;

            ret += val;
            if (it.hasNext()) {
                ret += ",";
            }
        }

        Log.d(TAG, ret);
        return ret;
    }

    public Cocos2dxGLSurfaceView onCreateView() {
        Cocos2dxGLSurfaceView glSurfaceView = new Cocos2dxGLSurfaceView(this);
        glSurfaceView.setEGLConfigChooser(5, 6, 5, 0, 16, 8);
        mGLView = glSurfaceView;
        return glSurfaceView;
    }

    protected void onDestroy() {
        if (adView != null) {
            adView.destroy();
        }

        super.onDestroy();
    }

    public static String loadUserId(Context context) {
        SharedPreferences preferences = PreferenceManager
                .getDefaultSharedPreferences(context);
        return preferences.getString(KEY_USERID, null);
    }

    public static void saveUserId(Context context, String userid) {
        SharedPreferences.Editor editor = PreferenceManager
                .getDefaultSharedPreferences(context).edit();
        editor.putString(KEY_USERID, userid);
        editor.commit();
    }
}
