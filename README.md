# DataPrivacy
数据加密库


Step 1：Add it in your root build.gradle at the end of repositories:

	allprojects {
		repositories {
			...
			maven { url 'https://www.jitpack.io' }
		}
	}
  
Step 2. Add the dependency

	dependencies {
	        compile 'com.github.changshuai7:DataPrivacy:1.0.0'
	}
	
	
Step 3. In your application init

	public class App extends Application {
	
             @Override
             public void onCreate() {
                 super.onCreate();
                 PrivacyUtils.getInstance().init(this);
          }
        }
	

Your can save the data：


	 PrivacyUtils.getInstance().setData("abc", PrivacyUtils.offset_1);
	 
	 
Your can get the data：

	
	String data = PrivacyUtils.getInstance().getData(PrivacyUtils.offset_1);
	
