function [Y,Cb,Cr] = rgb2ycbcr(rgb)

rgb = double(rgb);
R = rgb(:,:,1);
G = rgb(:,:,2);
B = rgb(:,:,3);

matrix = [0.299,0.587,0.114;0.5,-0.419, -0.081;-0.169,-0.331,0.500];

Y  = matrix(1,1)*R+matrix(1,2)*G+matrix(1,3)*B;
Cb = matrix(2,1)*R+matrix(2,2)*G+matrix(2,3)*B+128;
Cr = matrix(3,1)*R+matrix(3,2)*G+matrix(3,3)*B+128;