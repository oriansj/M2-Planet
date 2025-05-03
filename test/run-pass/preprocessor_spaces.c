#     if 0
#error Should never happen 1
#     elif 0
#error Should never happen 2
#endif

#     if 1
#   if 0
#error Should never happen 3
#endif
#  if 1
int main() { }
#  endif
#     endif
