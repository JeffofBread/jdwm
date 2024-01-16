static void
focusurgent(const Arg *arg) {
	Monitor *m;
	Client *c;
	int i;
	for(m=mons; m; m=m->next){
		for(c=m->clients; c && !c->isurgent; c=c->next);
		if(c) {
			unfocus(selmon->sel, 0);
			selmon = m;
			for(i=0; i < LENGTH(tags) && !((1 << i) & c->tags); i++);
			if(i < LENGTH(tags)) {
				const Arg a = {.ui = 1 << i};
				view(&a);
				focus(c);
				warp(c);
			}
		}
	}
}