#include <stdio.h>

// int gv;
// int gc = 2;

// int main(){
//     int lv;
//     int lc = 5;
//     gv = 1;
//     lv = 3;
//     printf("%d",gv);
//     printf("%d",gc);
//     printf("%d",lv);
//     printf("%d",lc);
//     return 0;
// }

int recursive(int index){
    if(index == 1){
        return 1;
    }
    else{
        return recursive(index - 1) + index;
    }
}

int main(){
    recursive(10);
}