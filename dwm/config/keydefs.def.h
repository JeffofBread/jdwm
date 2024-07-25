#include <X11/XF86keysym.h>

// Key Defenitions
#define Win Mod4Mask
#define Alt Mod1Mask
#define Shift ShiftMask
#define Control ControlMask
#define LeftClick Button1
#define MiddleClick Button2
#define RightClick Button3
#define PageUp XK_Prior
#define PageDown XK_Next

/* XFree86 vendor specific keysyms                                                                                      */
/*----------------------------------------------------------------------------------------------------------------------*/
/* Backlight controls.                                                                                                  */
#define XK_MonBrightnessUp              XF86XK_MonBrightnessUp          /* Monitor/panel brightness                     */
#define XK_MonBrightnessDown            XF86XK_MonBrightnessDown        /* Monitor/panel brightness                     */
#define XK_KbdLightOnOff                XF86XK_KbdLightOnOff            /* Keyboards may be lit                         */
#define XK_KbdBrightnessUp              XF86XK_KbdBrightnessUp          /* Keyboards may be lit                         */
#define XK_KbdBrightnessDown            XF86XK_KbdBrightnessDown        /* Keyboards may be lit                         */
#define XK_MonBrightnessCycle           XF86XK_MonBrightnessCycle       /* Monitor/panel brightness                     */
/*----------------------------------------------------------------------------------------------------------------------*/
/* Keys found on some "Internet" keyboards.                                                                             */
#define XK_Standby                      XF86XK_Standby                  /* System into standby mode                     */
#define XK_AudioLowerVolume             XF86XK_AudioLowerVolume         /* Volume control down                          */
#define XK_AudioMute                    XF86XK_AudioMute                /* Mute sound from the system                   */
#define XK_AudioRaiseVolume             XF86XK_AudioRaiseVolume         /* Volume control up                            */
#define XK_AudioPlay                    XF86XK_AudioPlay                /* Start playing of audio >                     */
#define XK_AudioStop                    XF86XK_AudioStop                /* Stop playing audio                           */
#define XK_AudioPrev                    XF86XK_AudioPrev                /* Previous track                               */
#define XK_AudioNext                    XF86XK_AudioNext                /* Next track                                   */
#define XK_HomePage                     XF86XK_HomePage                 /* Display user's home page                     */
#define XK_Mail                         XF86XK_Mail                     /* Invoke user's mail program                   */
#define XK_Start                        XF86XK_Start                    /* Start application                            */
#define XK_Search                       XF86XK_Search                   /* Search                                       */
#define XK_AudioRecord                  XF86XK_AudioRecord              /* Record audio application                     */
/*----------------------------------------------------------------------------------------------------------------------*/
/* These are sometimes found                                                                                            */
/* on PDA's (e.g. Palm, PocketPC or elsewhere)                                                                          */
#define XK_Calculator                   XF86XK_Calculator               /* Invoke calculator program                    */
#define XK_Memo                         XF86XK_Memo                     /* Invoke Memo taking program                   */
#define XK_ToDoList                     XF86XK_ToDoList                 /* Invoke To Do List program                    */
#define XK_Calendar                     XF86XK_Calendar                 /* Invoke Calendar program                      */
#define XK_PowerDown                    XF86XK_PowerDown                /* Deep sleep the system                        */
#define XK_ContrastAdjust               XF86XK_ContrastAdjust           /* Adjust screen contrast                       */
#define XK_RockerUp                     XF86XK_RockerUp                 /* Rocker switches exist up                     */
#define XK_RockerDown                   XF86XK_RockerDown               /* and down                                     */
#define XK_RockerEnter                  XF86XK_RockerEnter              /* and let you press them                       */
/*----------------------------------------------------------------------------------------------------------------------*/
/* Some more "Internet" keyboard symbols                                                                                */
#define XK_Back                         XF86XK_Back                     /* Like back on a browser                       */
#define XK_Forward                      XF86XK_Forward                  /* Like forward on a browser                    */
#define XK_Stop                         XF86XK_Stop                     /* Stop current operation                       */
#define XK_Refresh                      XF86XK_Refresh                  /* Refresh the page                             */
#define XK_PowerOff                     XF86XK_PowerOff                 /* Power off system entirely                    */
#define XK_WakeUp                       XF86XK_WakeUp                   /* Wake up system from sleep                    */
#define XK_Eject                        XF86XK_Eject                    /* Eject device (e.g. DVD)                      */
#define XK_ScreenSaver                  XF86XK_ScreenSaver              /* Invoke screensaver                           */
#define XK_WWW                          XF86XK_WWW                      /* Invoke web browser                           */
#define XK_Sleep                        XF86XK_Sleep                    /* Put system to sleep                          */
#define XK_Favorites                    XF86XK_Favorites                /* Show favorite locations                      */
#define XK_AudioPause                   XF86XK_AudioPause               /* Pause audio playing                          */
#define XK_AudioMedia                   XF86XK_AudioMedia               /* Launch media collection app                  */
#define XK_MyComputer                   XF86XK_MyComputer               /* Display "My Computer" window                 */
#define XK_VendorHome                   XF86XK_VendorHome               /* Display vendor home web site                 */
#define XK_LightBulb                    XF86XK_LightBulb                /* Light bulb keys exist                        */
#define XK_Shop                         XF86XK_Shop                     /* Display shopping web site                    */
#define XK_History                      XF86XK_History                  /* Show history of web surfing                  */
#define XK_OpenURL                      XF86XK_OpenURL                  /* Open selected URL                            */
#define XK_AddFavorite                  XF86XK_AddFavorite              /* Add URL to favorites list                    */
#define XK_HotLinks                     XF86XK_HotLinks                 /* Show "hot" links                             */
#define XK_BrightnessAdjust             XF86XK_BrightnessAdjust         /* Invoke brightness adj. UI                    */
#define XK_Finance                      XF86XK_Finance                  /* Display financial site                       */
#define XK_Community                    XF86XK_Community                /* Display user's community                     */
#define XK_AudioRewind                  XF86XK_AudioRewind              /* "rewind" audio track                         */
#define XK_BackForward                  XF86XK_BackForward              /* ???                                          */
#define XK_Launch0                      XF86XK_Launch0                  /* Launch Application                           */
#define XK_Launch1                      XF86XK_Launch1                  /* Launch Application                           */
#define XK_Launch2                      XF86XK_Launch2                  /* Launch Application                           */
#define XK_Launch3                      XF86XK_Launch3                  /* Launch Application                           */
#define XK_Launch4                      XF86XK_Launch4                  /* Launch Application                           */
#define XK_Launch5                      XF86XK_Launch5                  /* Launch Application                           */
#define XK_Launch6                      XF86XK_Launch6                  /* Launch Application                           */
#define XK_Launch7                      XF86XK_Launch7                  /* Launch Application                           */
#define XK_Launch8                      XF86XK_Launch8                  /* Launch Application                           */
#define XK_Launch9                      XF86XK_Launch9                  /* Launch Application                           */
#define XK_LaunchA                      XF86XK_LaunchA                  /* Launch Application                           */
#define XK_LaunchB                      XF86XK_LaunchB                  /* Launch Application                           */
#define XK_LaunchC                      XF86XK_LaunchC                  /* Launch Application                           */
#define XK_LaunchD                      XF86XK_LaunchD                  /* Launch Application                           */
#define XK_LaunchE                      XF86XK_LaunchE                  /* Launch Application                           */
#define XK_LaunchF                     XF86XK_LaunchF                   /* Launch Application                           */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_ApplicationLeft              XF86XK_ApplicationLeft          /* switch to application, left                  */
#define XK_ApplicationRight             XF86XK_ApplicationRight         /* switch to application, right                 */
#define XK_Book                         XF86XK_Book                     /* Launch bookreader                            */
#define XK_CD                           XF86XK_CD                       /* Launch CD/DVD player                         */
#define XK_Calculater                   XF86XK_Calculater               /* Launch Calculater                            */
//#define XK_Clear                        XF86XK_Clear                    /* Clear window, screen                         */
#define XK_Close                        XF86XK_Close                    /* Close window                                 */
#define XK_Copy                         XF86XK_Copy                     /* Copy selection                               */
#define XK_Cut                          XF86XK_Cut                      /* Cut selection                                */
#define XK_Display                      XF86XK_Display                  /* Output switch key                            */
#define XK_DOS                          XF86XK_DOS                      /* Launch DOS (emulation)                       */
#define XK_Documents                    XF86XK_Documents                /* Open documents window                        */
#define XK_Excel                        XF86XK_Excel                    /* Launch spread sheet                          */
#define XK_Explorer                     XF86XK_Explorer                 /* Launch file explorer                         */
#define XK_Game                         XF86XK_Game                     /* Launch game                                  */
#define XK_Go                           XF86XK_Go                       /* Go to URL                                    */
#define XK_iTouch                       XF86XK_iTouch                   /* Logitech iTouch- don't use                   */
#define XK_LogOff                       XF86XK_LogOff                   /* Log off system                               */
#define XK_Market                       XF86XK_Market                   /* ??                                           */
#define XK_Meeting                     XF86XK_Meeting                   /* enter meeting in calendar                    */
#define XK_MenuKB                       XF86XK_MenuKB                   /* distinguish keyboard from PB                 */
#define XK_MenuPB                       XF86XK_MenuPB                   /* distinguish PB from keyboard                 */
#define XK_MySites                      XF86XK_MySites                  /* Favourites                                   */
#define XK_New                          XF86XK_New                      /* New (folder, document...                     */
#define XK_News                         XF86XK_News                     /* News                                         */
#define XK_OfficeHome                   XF86XK_OfficeHome               /* Office home (old Staroffice)                 */
#define XK_Open                         XF86XK_Open                     /* Open                                         */
#define XK_Option                       XF86XK_Option                   /* ??                                           */
#define XK_Paste                        XF86XK_Paste                    /* Paste                                        */
#define XK_Phone                        XF86XK_Phone                    /* Launch phone; dial number                    */
//#define XK_Q                            XF86XK_Q                        /* Compaq's Q - don't use                       */
#define XK_Reply                        XF86XK_Reply                    /* Reply e.g., mail                             */
#define XK_Reload                       XF86XK_Reload                   /* Reload web page, file, etc.                  */
#define XK_RotateWindows                XF86XK_RotateWindows            /* Rotate windows e.g. xrandr                   */
#define XK_RotationPB                   XF86XK_RotationPB               /* don't use                                    */
#define XK_RotationKB                   XF86XK_RotationKB               /* don't use                                    */
#define XK_Save                         XF86XK_Save                     /* Save (file, document, state                  */
#define XK_ScrollUp                     XF86XK_ScrollUp                 /* Scroll window/contents up                    */
#define XK_ScrollDown                   XF86XK_ScrollDown               /* Scrool window/contentd down                  */
#define XK_ScrollClick                  XF86XK_ScrollClick              /* Use XKB mousekeys instead                    */
#define XK_Send                         XF86XK_Send                     /* Send mail, file, object                      */
#define XK_Spell                        XF86XK_Spell                    /* Spell checker                                */
#define XK_SplitScreen                  XF86XK_SplitScreen              /* Split window or screen                       */
#define XK_Support                      XF86XK_Support                  /* Get support (??)                             */
#define XK_TaskPane                     XF86XK_TaskPane                 /* Show tasks                                   */
#define XK_Terminal                     XF86XK_Terminal                 /* Launch terminal emulator                     */
#define XK_Tools                        XF86XK_Tools                    /* toolbox of desktop/app.                      */
#define XK_Travel                       XF86XK_Travel                   /* ??                                           */
#define XK_UserPB                       XF86XK_UserPB                   /* ??                                           */
#define XK_User1KB                      XF86XK_User1KB                  /* ??                                           */
#define XK_User2KB                      XF86XK_User2KB                  /* ??                                           */
#define XK_Video                        XF86XK_Video                    /* Launch video player                          */
#define XK_WheelButton                  XF86XK_WheelButton              /* button from a mouse wheel                    */
#define XK_Word                         XF86XK_Word                     /* Launch word processor                        */
#define XK_Xfer                         XF86XK_Xfer                     /*                                              */
#define XK_ZoomIn                       XF86XK_ZoomIn                   /* zoom in view, map, etc.                      */
#define XK_ZoomOut                      XF86XK_ZoomOut                  /* zoom out view, map, etc.                     */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_Away                         XF86XK_Away                     /* mark yourself as away                        */
#define XK_Messenger                    XF86XK_Messenger                /* as in instant messaging                      */
#define XK_WebCam                       XF86XK_WebCam                   /* Launch web camera app.                       */
#define XK_MailForward                  XF86XK_MailForward              /* Forward in mail                              */
#define XK_Pictures                     XF86XK_Pictures                 /* Show pictures                                */
#define XK_Music                        XF86XK_Music                    /* Launch music application                     */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_Battery                      XF86XK_Battery                  /* Display battery information                  */
#define XK_Bluetooth                    XF86XK_Bluetooth                /* Enable/disable Bluetooth                     */
#define XK_WLAN                         XF86XK_WLAN                     /* Enable/disable WLAN                          */
#define XK_UWB                          XF86XK_UWB                      /* Enable/disable UWB	                        */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_AudioForward                 XF86XK_AudioForward             /* fast-forward audio track                     */
#define XK_AudioRepeat                  XF86XK_AudioRepeat              /* toggle repeat mode                           */
#define XK_AudioRandomPlay              XF86XK_AudioRandomPlay          /* toggle shuffle mode                          */
#define XK_Subtitle                     XF86XK_Subtitle                 /* cycle through subtitle                       */
#define XK_AudioCycleTrack              XF86XK_AudioCycleTrack          /* cycle through audio tracks                   */
#define XK_CycleAngle                   XF86XK_CycleAngle               /* cycle through angles                         */
#define XK_FrameBack                    XF86XK_FrameBack                /* video: go one frame back                     */
#define XK_FrameForward                 XF86XK_FrameForward             /* video: go one frame forward                  */
#define XK_Time                         XF86XK_Time                     /* display, or shows an entry for time seeking  */
//#define XK_Select                       XF86XK_Select                   /* Select button on joypads and remotes         */
#define XK_View                         XF86XK_View                     /* Show a view options/properties               */
#define XK_TopMenu                      XF86XK_TopMenu                  /* Go to a top-level menu in a video            */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_Red                          XF86XK_Red                      /* Red button                                   */
#define XK_Green                        XF86XK_Green                    /* Green button                                 */
#define XK_Yellow                       XF86XK_Yellow                   /* Yellow button                                */
#define XK_Blue                         XF86XK_Blue                     /* Blue button                                  */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_Suspend                      XF86XK_Suspend                  /* Sleep to RAM                                 */
#define XK_Hibernate                    XF86XK_Hibernate                /* Sleep to disk                                */
#define XK_TouchpadToggle               XF86XK_TouchpadToggle           /* Toggle between touchpad/trackstick           */
#define XK_TouchpadOn                   XF86XK_TouchpadOn               /* The touchpad got switched on                 */
#define XK_TouchpadOff                  XF86XK_TouchpadOff              /* The touchpad got switched off                */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_AudioMicMute                 XF86XK_AudioMicMute             /* Mute the Mic from the system                 */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_Keyboard                     XF86XK_Keyboard                 /* User defined keyboard related action         */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_WWAN                         XF86XK_WWAN                     /* Toggle WWAN (LTE, UMTS, etc.) radio          */
#define XK_RFKill                       XF86XK_RFKill                   /* Toggle radios on/off                         */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_AudioPreset                  XF86XK_AudioPreset              /* Select equalizer preset, e.g. theatre-mode   */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_RotationLockToggle           XF86XK_RotationLockToggle       /* Toggle screen rotation lock on/off           */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_FullScreen                   XF86XK_FullScreen               /* Toggle fullscreen                            */
/*----------------------------------------------------------------------------------------------------------------------*/
/* Keys for special action keys (hot keys)                                                                              */
/* Virtual terminals on some operating systems                                                                          */
#define XK_Switch_VT_1                  XF86XK_Switch_VT_1              /*                                              */
#define XK_Switch_VT_2                  XF86XK_Switch_VT_2              /*                                              */
#define XK_Switch_VT_3                  XF86XK_Switch_VT_3              /*                                              */
#define XK_Switch_VT_4                  XF86XK_Switch_VT_4              /*                                              */
#define XK_Switch_VT_5                  XF86XK_Switch_VT_5              /*                                              */
#define XK_Switch_VT_6                  XF86XK_Switch_VT_6              /*                                              */
#define XK_Switch_VT_7                  XF86XK_Switch_VT_7              /*                                              */
#define XK_Switch_VT_8                  XF86XK_Switch_VT_8              /*                                              */
#define XK_Switch_VT_9                  XF86XK_Switch_VT_9              /*                                              */
#define XK_Switch_VT_10                 XF86XK_Switch_VT_10             /*                                              */
#define XK_Switch_VT_11                 XF86XK_Switch_VT_11             /*                                              */
#define XK_Switch_VT_12                 XF86XK_Switch_VT_12             /*                                              */
#define XK_Ungrab                       XF86XK_Ungrab                   /* force ungrab                                 */
#define XK_ClearGrab                    XF86XK_ClearGrab                /* kill application with grab                   */
#define XK_Next_VMode                   XF86XK_Next_VMode               /* next video mode available                    */
#define XK_Prev_VMode                   XF86XK_Prev_VMode               /* prev. video mode available                   */
#define XK_LogWindowTree                XF86XK_LogWindowTree            /* print window tree to log                     */
#define XK_LogGrabInfo                  XF86XK_LogGrabInfo              /* print all active grabs to log                */
/*----------------------------------------------------------------------------------------------------------------------*/
#define XK_BrightnessAuto               XF86XK_BrightnessAuto           /* v3.16    KEY_BRIGHTNESS_AUTO                 */
#define XK_DisplayOff                   XF86XK_DisplayOff               /* v2.6.23  KEY_DISPLAY_OFF                     */
#define XK_Info                         XF86XK_Info                     /*          KEY_INFO                            */
#define XK_AspectRatio                  XF86XK_AspectRatio              /* v5.1     KEY_ASPECT_RATIO                    */
#define XK_DVD                          XF86XK_DVD                      /*          KEY_DVD                             */
#define XK_Audio                        XF86XK_Audio                    /*          KEY_AUDIO                           */
#define XK_ChannelUp                    XF86XK_ChannelUp                /*          KEY_CHANNELUP                       */
#define XK_ChannelDown                  XF86XK_ChannelDown              /*          KEY_CHANNELDOWN                     */
//#define XK_Break                        XF86XK_Break                    /*          KEY_BREAK                           */
#define XK_VideoPhone                   XF86XK_VideoPhone               /* v2.6.20  KEY_VIDEOPHONE                      */
#define XK_ZoomReset                    XF86XK_ZoomReset                /* v2.6.20  KEY_ZOOMRESET                       */
#define XK_Editor                       XF86XK_Editor                   /* v2.6.20  KEY_EDITOR                          */
#define XK_GraphicsEditor               XF86XK_GraphicsEditor           /* v2.6.20  KEY_GRAPHICSEDITOR                  */
#define XK_Presentation                 XF86XK_Presentation             /* v2.6.20  KEY_PRESENTATION                    */
#define XK_Database                     XF86XK_Database                 /* v2.6.20  KEY_DATABASE                        */
#define XK_Voicemail                    XF86XK_Voicemail                /* v2.6.20  KEY_VOICEMAIL                       */
#define XK_Addressbook                  XF86XK_Addressbook              /* v2.6.20  KEY_ADDRESSBOOK                     */
#define XK_DisplayToggle                XF86XK_DisplayToggle            /* v2.6.20  KEY_DISPLAYTOGGLE                   */
#define XK_SpellCheck                   XF86XK_SpellCheck               /* v2.6.24  KEY_SPELLCHECK                      */
#define XK_ContextMenu                  XF86XK_ContextMenu              /* v2.6.24  KEY_CONTEXT_MENU                    */
#define XK_MediaRepeat                  XF86XK_MediaRepeat              /* v2.6.26  KEY_MEDIA_REPEAT                    */
#define XK_10ChannelsUp                 XF86XK_10ChannelsUp             /* v2.6.38  KEY_10CHANNELSUP                    */
#define XK_10ChannelsDown               XF86XK_10ChannelsDown           /* v2.6.38  KEY_10CHANNELSDOWN                  */
#define XK_Images                       XF86XK_Images                   /* v2.6.39  KEY_IMAGES                          */
#define XK_NotificationCenter           XF86XK_NotificationCenter       /* v5.10    KEY_NOTIFICATION_CENTER             */
#define XK_PickupPhone                  XF86XK_PickupPhone              /* v5.10    KEY_PICKUP_PHONE                    */
#define XK_HangupPhone                  XF86XK_HangupPhone              /* v5.10    KEY_HANGUP_PHONE                    */
#define XK_Fn                           XF86XK_Fn                       /*          KEY_FN                              */
#define XK_Fn_Esc                       XF86XK_Fn_Esc                   /*          KEY_FN_ESC                          */
#define XK_FnRightShift                 XF86XK_FnRightShift             /* v5.10    KEY_FN_RIGHT_SHIFT                  */
#define XK_Numeric0                     XF86XK_Numeric0                 /* v2.6.28  KEY_NUMERIC_0                       */
#define XK_Numeric1                     XF86XK_Numeric1                 /* v2.6.28  KEY_NUMERIC_1                       */
#define XK_Numeric2                     XF86XK_Numeric2                 /* v2.6.28  KEY_NUMERIC_2                       */
#define XK_Numeric3                     XF86XK_Numeric3                 /* v2.6.28  KEY_NUMERIC_3                       */
#define XK_Numeric4                     XF86XK_Numeric4                 /* v2.6.28  KEY_NUMERIC_4                       */
#define XK_Numeric5                     XF86XK_Numeric5                 /* v2.6.28  KEY_NUMERIC_5                       */
#define XK_Numeric6                     XF86XK_Numeric6                 /* v2.6.28  KEY_NUMERIC_6                       */
#define XK_Numeric7                     XF86XK_Numeric7                 /* v2.6.28  KEY_NUMERIC_7                       */
#define XK_Numeric8                     XF86XK_Numeric8                 /* v2.6.28  KEY_NUMERIC_8                       */
#define XK_Numeric9                     XF86XK_Numeric9                 /* v2.6.28  KEY_NUMERIC_9                       */
#define XK_NumericStar                  XF86XK_NumericStar              /* v2.6.28  KEY_NUMERIC_STAR                    */
#define XK_NumericPound                 XF86XK_NumericPound             /* v2.6.28  KEY_NUMERIC_POUND                   */
#define XK_NumericA                     XF86XK_NumericA                 /* v4.1     KEY_NUMERIC_A                       */
#define XK_NumericB                     XF86XK_NumericB                 /* v4.1     KEY_NUMERIC_B                       */
#define XK_NumericC                     XF86XK_NumericC                 /* v4.1     KEY_NUMERIC_C                       */
#define XK_NumericD                     XF86XK_NumericD                 /* v4.1     KEY_NUMERIC_D                       */
#define XK_CameraFocus                  XF86XK_CameraFocus              /* v2.6.33  KEY_CAMERA_FOCUS                    */
#define XK_WPSButton                    XF86XK_WPSButton                /* v2.6.34  KEY_WPS_BUTTON                      */
#define XK_CameraZoomIn                 XF86XK_CameraZoomIn             /* v2.6.39  KEY_CAMERA_ZOOMIN                   */
#define XK_CameraZoomOut                XF86XK_CameraZoomOut            /* v2.6.39  KEY_CAMERA_ZOOMOUT                  */
#define XK_CameraUp                     XF86XK_CameraUp                 /* v2.6.39  KEY_CAMERA_UP                       */
#define XK_CameraDown                   XF86XK_CameraDown               /* v2.6.39  KEY_CAMERA_DOWN                     */
#define XK_CameraLeft                   XF86XK_CameraLeft               /* v2.6.39  KEY_CAMERA_LEFT                     */
#define XK_CameraRight                  XF86XK_CameraRight              /* v2.6.39  KEY_CAMERA_RIGHT                    */
#define XK_AttendantOn                  XF86XK_AttendantOn              /* v3.10    KEY_ATTENDANT_ON                    */
#define XK_AttendantOff                 XF86XK_AttendantOff             /* v3.10    KEY_ATTENDANT_OFF                   */
#define XK_AttendantToggle              XF86XK_AttendantToggle          /* v3.10    KEY_ATTENDANT_TOGGLE                */
#define XK_LightsToggle                 XF86XK_LightsToggle             /* v3.10    KEY_LIGHTS_TOGGLE                   */
#define XK_ALSToggle                    XF86XK_ALSToggle                /* v3.13    KEY_ALS_TOGGLE                      */
#define XK_Buttonconfig                 XF86XK_Buttonconfig             /* v3.16    KEY_BUTTONCONFIG                    */
#define XK_Taskmanager                  XF86XK_Taskmanager              /* v3.16    KEY_TASKMANAGER                     */
#define XK_Journal                      XF86XK_Journal                  /* v3.16    KEY_JOURNAL                         */
#define XK_ControlPanel                 XF86XK_ControlPanel             /* v3.16    KEY_CONTROLPANEL                    */
//#define XK_AppSelect                    XF86XK_AppSelect                /* v3.16    KEY_APPSELECT                       */
#define XK_Screensaver                  XF86XK_Screensaver              /* v3.16    KEY_SCREENSAVER                     */
#define XK_VoiceCommand                 XF86XK_VoiceCommand             /* v3.16    KEY_VOICECOMMAND                    */
#define XK_Assistant                    XF86XK_Assistant                /* v4.13    KEY_ASSISTANT                       */
#define XK_EmojiPicker                  XF86XK_EmojiPicker              /* v5.13    KEY_EMOJI_PICKER                    */
#define XK_Dictate                      XF86XK_Dictate                  /* v5.17    KEY_DICTATE                         */
#define XK_BrightnessMin                XF86XK_BrightnessMin            /* v3.16    KEY_BRIGHTNESS_MIN                  */
#define XK_BrightnessMax                XF86XK_BrightnessMax            /* v3.16    KEY_BRIGHTNESS_MAX                  */
#define XK_KbdInputAssistPrev           XF86XK_KbdInputAssistPrev       /* v3.18    KEY_KBDINPUTASSIST_PREV             */
#define XK_KbdInputAssistNext           XF86XK_KbdInputAssistNext       /* v3.18    KEY_KBDINPUTASSIST_NEXT             */
#define XK_KbdInputAssistPrevgroup      XF86XK_KbdInputAssistPrevgroup  /* v3.18    KEY_KBDINPUTASSIST_PREVGROUP        */
#define XK_KbdInputAssistNextgroup      XF86XK_KbdInputAssistNextgroup  /* v3.18    KEY_KBDINPUTASSIST_NEXTGROUP        */
#define XK_KbdInputAssistAccept         XF86XK_KbdInputAssistAccept     /* v3.18    KEY_KBDINPUTASSIST_ACCEPT           */
#define XK_KbdInputAssistCancel         XF86XK_KbdInputAssistCancel     /* v3.18    KEY_KBDINPUTASSIST_CANCEL           */
#define XK_RightUp                      XF86XK_RightUp                  /* v4.7     KEY_RIGHT_UP                        */
#define XK_RightDown                    XF86XK_RightDown                /* v4.7     KEY_RIGHT_DOWN                      */
#define XK_LeftUp                       XF86XK_LeftUp                   /* v4.7     KEY_LEFT_UP                         */
#define XK_LeftDown                     XF86XK_LeftDown                 /* v4.7     KEY_LEFT_DOWN                       */
#define XK_RootMenu                     XF86XK_RootMenu                 /* v4.7     KEY_ROOT_MENU                       */
#define XK_MediaTopMenu                 XF86XK_MediaTopMenu             /* v4.7     KEY_MEDIA_TOP_MENU                  */
#define XK_Numeric11                    XF86XK_Numeric11                /* v4.7     KEY_NUMERIC_11                      */
#define XK_Numeric12                    XF86XK_Numeric12                /* v4.7     KEY_NUMERIC_12                      */
#define XK_AudioDesc                    XF86XK_AudioDesc                /* v4.7     KEY_AUDIO_DESC                      */
#define XK_3DMode                       XF86XK_3DMode                   /* v4.7     KEY_3D_MODE                         */
#define XK_NextFavorite                 XF86XK_NextFavorite             /* v4.7     KEY_NEXT_FAVORITE                   */
#define XK_StopRecord                   XF86XK_StopRecord               /* v4.7     KEY_STOP_RECORD                     */
#define XK_PauseRecord                  XF86XK_PauseRecord              /* v4.7     KEY_PAUSE_RECORD                    */
#define XK_VOD                          XF86XK_VOD                      /* v4.7     KEY_VOD                             */
#define XK_Unmute                       XF86XK_Unmute                   /* v4.7     KEY_UNMUTE                          */
#define XK_FastReverse                  XF86XK_FastReverse              /* v4.7     KEY_FASTREVERSE                     */
#define XK_SlowReverse                  XF86XK_SlowReverse              /* v4.7     KEY_SLOWREVERSE                     */
#define XK_Data                         XF86XK_Data                     /* v4.7     KEY_DATA                            */
#define XK_OnScreenKeyboard             XF86XK_OnScreenKeyboard         /* v4.12    KEY_ONSCREEN_KEYBOARD               */
#define XK_PrivacyScreenToggle          XF86XK_PrivacyScreenToggle      /* v5.5     KEY_PRIVACY_SCREEN_TOGGLE           */
#define XK_SelectiveScreenshot          XF86XK_SelectiveScreenshot      /* v5.6     KEY_SELECTIVE_SCREENSHOT            */
#define XK_Macro1                       XF86XK_Macro1                   /* v5.5     KEY_MACRO1                          */
#define XK_Macro2                       XF86XK_Macro2                   /* v5.5     KEY_MACRO2                          */
#define XK_Macro3                       XF86XK_Macro3                   /* v5.5     KEY_MACRO3                          */
#define XK_Macro4                       XF86XK_Macro4                   /* v5.5     KEY_MACRO4                          */
#define XK_Macro5                       XF86XK_Macro5                   /* v5.5     KEY_MACRO5                          */
#define XK_Macro6                       XF86XK_Macro6                   /* v5.5     KEY_MACRO6                          */
#define XK_Macro7                       XF86XK_Macro7                   /* v5.5     KEY_MACRO7                          */
#define XK_Macro8                       XF86XK_Macro8                   /* v5.5     KEY_MACRO8                          */
#define XK_Macro9                       XF86XK_Macro9                   /* v5.5     KEY_MACRO9                          */
#define XK_Macro10                      XF86XK_Macro10                  /* v5.5     KEY_MACRO10                         */
#define XK_Macro11                      XF86XK_Macro11                  /* v5.5     KEY_MACRO11                         */
#define XK_Macro12                      XF86XK_Macro12                  /* v5.5     KEY_MACRO12                         */
#define XK_Macro13                      XF86XK_Macro13                  /* v5.5     KEY_MACRO13                         */
#define XK_Macro14                      XF86XK_Macro14                  /* v5.5     KEY_MACRO14                         */
#define XK_Macro15                      XF86XK_Macro15                  /* v5.5     KEY_MACRO15                         */
#define XK_Macro16                      XF86XK_Macro16                  /* v5.5     KEY_MACRO16                         */
#define XK_Macro17                      XF86XK_Macro17                  /* v5.5     KEY_MACRO17                         */
#define XK_Macro18                      XF86XK_Macro18                  /* v5.5     KEY_MACRO18                         */
#define XK_Macro19                      XF86XK_Macro19                  /* v5.5     KEY_MACRO19                         */
#define XK_Macro20                      XF86XK_Macro20                  /* v5.5     KEY_MACRO20                         */
#define XK_Macro21                      XF86XK_Macro21                  /* v5.5     KEY_MACRO21                         */
#define XK_Macro22                      XF86XK_Macro22                  /* v5.5     KEY_MACRO22                         */
#define XK_Macro23                      XF86XK_Macro23                  /* v5.5     KEY_MACRO23                         */
#define XK_Macro24                      XF86XK_Macro24                  /* v5.5     KEY_MACRO24                         */
#define XK_Macro25                      XF86XK_Macro25                  /* v5.5     KEY_MACRO25                         */
#define XK_Macro26                      XF86XK_Macro26                  /* v5.5     KEY_MACRO26                         */
#define XK_Macro27                      XF86XK_Macro27                  /* v5.5     KEY_MACRO27                         */
#define XK_Macro28                      XF86XK_Macro28                  /* v5.5     KEY_MACRO28                         */
#define XK_Macro29                      XF86XK_Macro29                  /* v5.5     KEY_MACRO29                         */
#define XK_Macro30                      XF86XK_Macro30                  /* v5.5     KEY_MACRO30                         */
#define XK_MacroRecordStart             XF86XK_MacroRecordStart         /* v5.5     KEY_MACRO_RECORD_START              */
#define XK_MacroRecordStop              XF86XK_MacroRecordStop          /* v5.5     KEY_MACRO_RECORD_STOP               */
#define XK_MacroPresetCycle             XF86XK_MacroPresetCycle         /* v5.5     KEY_MACRO_PRESET_CYCLE              */
#define XK_MacroPreset1                 XF86XK_MacroPreset1             /* v5.5     KEY_MACRO_PRESET1                   */
#define XK_MacroPreset2                 XF86XK_MacroPreset2             /* v5.5     KEY_MACRO_PRESET2                   */
#define XK_MacroPreset3                 XF86XK_MacroPreset3             /* v5.5     KEY_MACRO_PRESET3                   */
#define XK_KbdLcdMenu1                  XF86XK_KbdLcdMenu1              /* v5.5     KEY_KBD_LCD_MENU1                   */
#define XK_KbdLcdMenu2                  XF86XK_KbdLcdMenu2              /* v5.5     KEY_KBD_LCD_MENU2                   */
#define XK_KbdLcdMenu3                  XF86XK_KbdLcdMenu3              /* v5.5     KEY_KBD_LCD_MENU3                   */
#define XK_KbdLcdMenu4                  XF86XK_KbdLcdMenu4              /* v5.5     KEY_KBD_LCD_MENU4                   */
#define XK_KbdLcdMenu5                  XF86XK_KbdLcdMenu5              /* v5.5     KEY_KBD_LCD_MENU5                   */
/*----------------------------------------------------------------------------------------------------------------------*/