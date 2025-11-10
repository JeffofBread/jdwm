#ifndef COMMON_H_
#define COMMON_H_

#define MAX( A, B ) ( ( A ) > ( B ) ? ( A ) : ( B ) )
#define MIN( A, B ) ( ( A ) < ( B ) ? ( A ) : ( B ) )
#define LENGTH( X ) ( sizeof X / sizeof X[0] )
#define LENGTH_INT( X ) (int) LENGTH( X )
#define WIDTH( X ) ( ( X )->width + 2 * ( X )->border_width )
#define HEIGHT( X ) ( ( X )->height + 2 * ( X )->border_width )

typedef union Argument {
        int i;
        unsigned int ui;
        float f;
        const void *v;
} Argument_t;

#endif /* COMMON_H_ */