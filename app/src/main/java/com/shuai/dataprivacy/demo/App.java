package com.shuai.dataprivacy.demo;

import android.app.Application;

import com.shuai.dataprivacy.PrivacyUtils;

/**
 * Created by changshuai on 2018/3/20.
 */

public class App extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        PrivacyUtils.getInstance().init(this);
    }
}
