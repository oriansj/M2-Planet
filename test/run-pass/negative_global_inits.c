
int global = -1;

struct {
    int a;
} global_struct = { -1 };

int global_array[2] = { -1, -1 };

int main() {
    if(global != -1) return 1;

    if(global_struct.a != -1) return 2;

    if(global_array[0] != -1) return 3;
    if(global_array[1] != -1) return 4;

    int local = -1;
    if(local != -1) return 5;
}
