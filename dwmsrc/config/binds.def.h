static const Key keys[] = {
	/* modifier                     key        function        		argument */
	{ MODKEY,                       XK_d,      spawn,          		{.v = rofi_launcher_cmd } },
	{ MODKEY,                       XK_Return, spawn,          		{.v = termcmd } },
	{ ControlMask|ShiftMask,        XK_q,      spawn,          		{.v = recompilecmd } },
	{ MODKEY,                       XK_o,      shiftboth,      		{ .i = +1 } },
	{ MODKEY|ShiftMask,             XK_o,	   shiftview,      		{ .i = +1 } },
	{ MODKEY|ShiftMask,             XK_i,	   shiftview,      		{ .i = -1 } },
	{ MODKEY,	                    XK_i,      shiftboth,      		{ .i = -1 } },
	{ MODKEY|ShiftMask,             XK_b,      togglebar,      		{0} },
	{ MODKEY,                       XK_Up,     focusstack,     		{.i = +1 } },
	{ MODKEY,                       XK_Down,   focusstack,     		{.i = -1 } },
	{ MODKEY|ControlMask,           XK_i,      incnmaster,     		{.i = +1 } },
	{ MODKEY|ControlMask,           XK_d,      incnmaster,     		{.i = -1 } },
	{ MODKEY|ShiftMask,		        XK_h,      shiftviewclients,	{ .i = -1 }	},
	{ MODKEY|ControlMask,	     	XK_h,      shiftswaptags,  		{ .i = -1 }	},
	{ MODKEY|ControlMask,	    	XK_l,      shiftswaptags,  		{ .i = +1 }	},
	{ MODKEY|ShiftMask,             XK_l,      shiftviewclients,	{ .i = +1 }	},
	{ MODKEY,                       XK_h,      setmfact,       		{.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       		{.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           		{0} },
	{ MODKEY|Mod1Mask,              XK_u,      incrgaps,       		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_u,      incrgaps,       		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_i,      incrigaps,      		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_i,      incrigaps,      		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_o,      incrogaps,      		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_o,      incrogaps,      		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_6,      incrihgaps,     		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_6,      incrihgaps,     		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_7,      incrivgaps,     		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_7,      incrivgaps,     		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_8,      incrohgaps,     		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_8,      incrohgaps,     		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_9,      incrovgaps,     		{.i = +1 } },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_9,      incrovgaps,     		{.i = -1 } },
	{ MODKEY|Mod1Mask,              XK_0,      togglegaps,     		{0} },
	{ MODKEY|Mod1Mask|ShiftMask,    XK_0,      defaultgaps,    		{0} },
	{ MODKEY,			            XK_Tab,    view,           		{0} },
	{ MODKEY,                       XK_q,      killclient,     		{0} },
	{ MODKEY,                       XK_t,      setlayout,      		{.v = &layouts[0]} },
	{ MODKEY,                       XK_e,      setlayout,      		{.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      		{.v = &layouts[2]} },
	{ MODKEY|ControlMask,           XK_j,      rotatestack,    		{.i = +1 } },
	{ MODKEY|ControlMask,           XK_k,      rotatestack,    		{.i = -1 } },
	{ MODKEY,                       XK_f,      togglefullscreen, 	{0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefakefullscreen,{0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, 		{0} },
	{ MODKEY,                       XK_0,      view,           		{.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            		{.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_Left,   tagmon,         		{.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,  tagmon,         		{.i = +1 } },
	{ MODKEY|ShiftMask,             XK_Left,   focusmon,       		{.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Right,  focusmon,       		{.i = +1 } },
	{ MODKEY,                       XK_Left,   focusmon,       		{.i = -1 } },
	{ MODKEY,                       XK_Right,  focusmon,       		{.i = +1 } },
	{ MODKEY,                       XK_n,      togglealttag,   		{0} },
	TAGKEYS(                        XK_1,                      		0)
	TAGKEYS(                        XK_2,                      		1)
	TAGKEYS(                        XK_3,                     		2)
	TAGKEYS(                        XK_4,                      		3)
	TAGKEYS(                        XK_5,                      		4)
	TAGKEYS(                        XK_6,                      		5)
	TAGKEYS(                        XK_7,                      		6)
	TAGKEYS(                        XK_8,                      		7)
	TAGKEYS(                        XK_9,                      		8)
	{ MODKEY|ShiftMask,             XK_q,      quit,    	   		{1} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           		{0} },
	//{ MODKEY|ShiftMask,             XK_e,      exitdwm,        		{0} },
	{ MODKEY|ShiftMask,             XK_u,      focusurgent,    		{0} },
	{ MODKEY,                       XK_s,      togglesticky,    	{0} },
	{ MODKEY,                       XK_u,      swalstopsel,    		{0} },
	{ MODKEY,                       XK_grave,  togglescratch,       {.v = scratchpadcmd } },
	{ Mod1Mask,                     XK_Tab,    spawn,               {.v = alttabcmd } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        spawn,          {.v = layoutmenucmd} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },

	// Note for myself on a future patch:
	// When you go to add the notitle patch, make it on an int toggle in config.h
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	/* placemouse options, choose which feels more natural:
	 *    0 - tiled position is relative to mouse cursor
	 *    1 - tiled postiion is relative to window center
	 *    2 - mouse pointer warps to window center
	 *
	 * The moveorplace uses movemouse or placemouse depending on the floating state
	 * of the selected client. Set up individual keybindings for the two if you want
	 * to control these separately (i.e. to retain the feature to move a tiled window
	 * into a floating position).
	 */
	{ ClkClientWin,      MODKEY,          Button1,        moveorplace,    {.i = 0} },
	{ ClkClientWin,      MODKEY,          Button2,        togglefloating, {0} },
	{ ClkClientWin,      MODKEY,          Button3,        resizemouse,    {0} },
	{ ClkClientWin,      MODKEY|ShiftMask,Button1,        swalmouse,      {0} },
	{ ClkTagBar,         0,               Button1,        view,           {0} },
	{ ClkTagBar,         0,               Button3,        toggleview,     {0} },
	{ ClkTagBar,         MODKEY,          Button1,        tag,            {0} },
	{ ClkTagBar,         MODKEY,          Button3,        toggletag,      {0} },
	//{ ClkRootWin,  		 0, 			  Button3, 		  spawn, 		  {.v = jgmenucmd } },
};