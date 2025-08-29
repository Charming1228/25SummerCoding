#include <iostream>
#include <stdlib.h>

using namespace std;

// 判断当前8个皇后的位置是否互不攻击
// 参数i1~i8分别表示第1~8列皇后所在的行号（1~8）
bool isValid(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8) {
    int positions[8] = {i1, i2, i3, i4, i5, i6, i7, i8};
    // 检查任意两个皇后是否冲突
    for (int col1 = 0; col1 < 8; col1++) {
        for (int col2 = col1 + 1; col2 < 8; col2++) {
            // 检查是否在同一行
            if (positions[col1] == positions[col2]) {
                return false;
            }
            // 检查是否在同一对角线
            if (abs(positions[col1] - positions[col2]) == abs(col1 - col2)) {
                return false;
            }
        }
    }
    return true; // 没有冲突，合法
}

int main(int argc, char *argv[])
{
    int Num = 0; // 记录解的个数
    // 8重循环，枚举每一列皇后所在的行
    for(int i1=1;i1<=8;i1++){
        for(int i2=1;i2<=8;i2++){
            for(int i3=1;i3<=8;i3++){
                for(int i4=1;i4<=8;i4++){
                    for(int i5=1;i5<=8;i5++){
                        for(int i6=1;i6<=8;i6++){
                            for(int i7=1;i7<=8;i7++){
                                for(int i8=1;i8<=8;i8++){
                                    // 检查当前摆法是否合法
                                    if(isValid(i1,i2,i3,i4,i5,i6,i7,i8)){
                                        Num++; // 合法解+1
                                        // 输出当前解
                                        printf("solution %d: %d %d %d %d %d %d %d %d\n", Num, i1, i2, i3, i4, i5, i6, i7, i8);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
