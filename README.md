# jdwm | homepage

jdwm is a custom build of [dwm](https://dwm.suckless.org/) made by myself, JeffofBread. This is simply the homepage to help you choose and navigate to a branch as well as get a simple overview of the project as a whole. 

<img src="https://github.com/JeffofBread/jdwm/blob/screenshots/jdwm_storm.png"> 
<img src="https://github.com/JeffofBread/jdwm/blob/screenshots/jdwm_lush.png">

## Branches

There are two main branches of jdwm, [`jdwm_pure`](https://github.com/JeffofBread/jdwm/tree/jdwm_pure) and [`jdwm_complete`](https://github.com/JeffofBread/jdwm/tree/jdwm_complete). 

[`jdwm_pure`](https://github.com/JeffofBread/jdwm/tree/jdwm_pure) is a stripped down and less integrated build of jdwm intended for those wanting a closer to 'base' dwm experience. It is intended for more experienced users who just want the patches and fixes jdwm has with none of the fluff of dependencies or scripts I have added alongside the base program.

[`jdwm_complete`](https://github.com/JeffofBread/jdwm/tree/jdwm_complete) however is the 'intended' or *complete* build of jdwm, with all of the scripts, assisting programs, and other features that really flesh out the experience. This is the build I would recommend to most anyone that is wanting to try jdwm.

[`patches`](https://github.com/JeffofBread/jdwm/tree/patches) is a branch dedicated to cataloging all the dwm patches used in jdwm and to credit their authors appropriately. This branch has little practical use for most users. 

[`screenshots`](https://github.com/JeffofBread/jdwm/tree/screenshots) is a branch that exists solely to hold screenshots of jdwm without them cluttering the main repository. This way they can be used in README (like you see above) without the user ever cloning them and using up bandwidth during download or unneeded space on their computer. Has little practical use for most users. 

[`wallpapers`](https://github.com/JeffofBread/jdwm/tree/wallpapers) is a branch that exists to hold wallpapers used by [`jdwm_complete`](https://github.com/JeffofBread/jdwm/tree/jdwm_complete), again to avoid cluttering the main repository and providing flexibility over whether the user wishes to download them or not. These wallpapers are used and defined in theme header files located in `/jdwm/dwm/themes/`, and the repository is cloned by default by `install.sh`. 

For more info on jdwm or a specific branch, check them out and read their specific README

## The future of jdwm

## Future

I still have many plans for changes and features, but currently don't have that much time to put towards this project, so don't expect too many updates, at least not rapid ones. If you would like to suggest ideas, features, or contribute, please either reach out on discord @jeffofbread, start a discussion here on github, or create a pull request. 

Edit | Oct 16th, 2024: As mentioned above, I currently don't have a whole lot of time to put towards the project, but
wanted to give some insight into what is being worked on. I am working on a complete rewrite of jdwm. Simply put, a lot
of jdwm was cobbled together quite poorly and there was a lot of self created tech debt in the project. On top of the
rewrite I am also creating complete documentation for the project, and overhauling how many of the scripts and other
tooling work surrounding jdwm. It is a massive undertaking and has taken far longer than expected, and will likely
still be another couple months before I am done (at the current pace). However, it will be massively beneficial in
the long run, and will allow for a lot of what I have planned for the project to be possible in the future.
Thank you to anyone following the project, and feel free to reach out if would like to help me test the new build (discord @jeffofbread). 

Edit | April 25th, 2025: I'll add another update here since it's been a few months without anything posted here. The project is very 
much still being worked on, but with so many radical and breaking changes patch to patch, I don't really want to post it much. As it
stands, my  personal development repository of this project sits at over 1,000 commits ahead of this repository, with innumerable 
changes to just about every single part of this project. I have poured probably close to five or six hundred hours of development 
time into the dev build at this point. I would have an easier time listing the few things that are the same between the two. It is 
taking me much longer than I anticipated because of one, my naivety. I simply misjudged how long some of the changes I had in mind 
would take. Second, and partially related to the first, has revolved around fixing bugs and issues that have either existed in jdwm, 
dwm, the patches (and my implementations of them), or fixing issues created changes I am making now to jdwm, like issues related to 
file structure and architecture. Finally, scope creep has massively delayed the project. During all these changes, it has been easy 
to decide that a new feature or modification should be done, because "while I'm here, I might as well...". All this being said, I 
will not bother to predict when the final build will be released here. If you are interested in trying the dev build, I would love 
people to test it. Please reach out to be on discord @jeffofbread, I'm more than happy to provide you a copy of the dev build and 
help you get set up with it.

