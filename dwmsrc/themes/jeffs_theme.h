// Jeff's custom color scheme, very simple green scheme
static const char greygreen[]   = "#2c4c3b";    // SchemeSel
static const char normalgreen[] = "#253a29";    // Bar background
static const char darkgreen[]   = "#182c25";    // SchemeNorm
static const char bluegreen[]   = "#1e453e";    // Unused
static const char grey[]        = "#455b55";    // Unused
static const char white[]       = "#e1e0db";    // Text


static const char *colors[][4] = {
    //               fg         bg          border      float
    [SchemeNorm] = { darkgreen, darkgreen,  darkgreen,  darkgreen },    // Generic scheme
    [SchemeSel]  = { greygreen, greygreen,  greygreen,  greygreen },    // Generic scheme for focused windows

    // colorbar Patch
    [SchemeSystray]     = { white,          normalgreen,      "#000000"  },   // Systray {text,background,not used but cannot be empty}
    [SchemeStatus]      = { white,          normalgreen,      "#000000"  },   // Statusbar right {text,background,not used but cannot be empty}
    [SchemeTagsSel]     = { white,          normalgreen,      "#000000"  },   // Tagbar left selected {text,background,not used but cannot be empty}
    [SchemeTagsNorm]    = { white,          normalgreen,      "#000000"  },   // Tagbar left unselected {text,background,not used but cannot be empty}
    [SchemeInfoSel]     = { white,          normalgreen,      "#000000"  },   // infobar middle  selected {text,background,not used but cannot be empty}
    [SchemeInfoNorm]    = { white,          normalgreen,      "#000000"  },   // infobar middle  unselected {text,background,not used but cannot be empty}
};