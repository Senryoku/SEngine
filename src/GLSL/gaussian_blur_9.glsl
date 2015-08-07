const int kernel_size = 9;
const int kernel_radius = kernel_size / 2;

uniform int offset[kernel_radius + 1] = int[](0, 1, 2, 3, 4);
uniform float weight[kernel_radius + 1] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
