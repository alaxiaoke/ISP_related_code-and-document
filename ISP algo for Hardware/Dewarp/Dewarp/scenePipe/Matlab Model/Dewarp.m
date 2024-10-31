clear
clc
close all;

img = double(imread('20220830-160918.bmp'));
[height,width,dim] = size(img);
data = load('20220830-160918.bmp_scale_+74.txt');

datax = data(:,3);
datay = data(:,4);
mapx1 = reshape(datax,[width,height]);
mapx = mapx1';
mapy1 = reshape(datay,[width,height]);
mapy = mapy1';

Dst = img;
for i=1:height
    for j=1:width
        coory = mapy(i,j)+1;
        coorx = mapx(i,j)+1;
        h1 = floor(coory);
        h2 =h1+1;
        gainh = coory-h1;
        
        w1 = floor(coorx);
        w2 =w1+1;
        gainw = coorx-w1;
        
        p11 = img(h1,w1,:);
        p12 = img(h1,w2,:);
        p21 = img(h2,w1,:);
        p22 = img(h2,w2,:);
        
        temp1 = gainh.*p21+(1-gainh).*p11;
        temp2 = gainh.*p22+(1-gainh).*p12;
        result = gainw.*temp2+(1-gainw).*temp1;
        Dst(i,j,:) = result;
    end
end
figure;imshow(uint8(img));title('distort Image')
figure;imshow(uint8(Dst));title('Un distort Image')