clear
clc
close all;

hd = 800;
wd = 1280;

img = double(imread('20220830-160918.bmp'));

img1 = imresize(img,[hd wd],'bicubic');
figure;imshow(uint8(img))
figure;imshow(uint8(img1))
[height,width,dim] = size(img);

[Y,Cb,Cr] = rgb2ycbcr(img1);
YYY = Y.*4;
fid = fopen('input_1280x800_mono.raw','wb');
fwrite(fid,YYY','uint16');
fclose(fid);

RatioH = height/hd;
RatioW = width/wd;
data = load('20220830-160918.bmp_scale_+74.txt');

datax = data(:,3);
datay = data(:,4);
mapx1 = reshape(datax,[width,height]);
mapx = mapx1';
mapy1 = reshape(datay,[width,height]);
mapy = mapy1';

mapxd = imresize(mapx,[hd,wd],'bilinear');
mapyd = imresize(mapy,[hd,wd],'bilinear');

mapxd1 =mapxd./RatioW;
mapyd1 =mapyd./RatioH;

Dst = Y;
for i=1:hd
    for j=1:wd
        coory = mapyd1(i,j)+1;
        coorx = mapxd1(i,j)+1;
        h1 = floor(coory);
        h2 =h1+1;
        gainh = coory-h1;
        
        w1 = floor(coorx);
        w2 =w1+1;
        gainw = coorx-w1;
        
        p11 = img1(h1,w1);
        p12 = img1(h1,w2);
        p21 = img1(h2,w1);
        p22 = img1(h2,w2);
        
        temp1 = gainh*p21+(1-gainh)*p11;
        temp2 = gainh*p22+(1-gainh)*p12;
        result = gainw*temp2+(1-gainw)*temp1;
        Dst(i,j) = result;
    end
end
% figure;imshow(uint8(img1));title('distort Image')
figure;imshow(uint8(Dst));title('Un distort Image')

fidx = fopen('mapx_mono.txt','wb+');
fidy = fopen('mapy_mono.txt','wb+');
for i=1:hd
    for j=1:wd
        fprintf(fidx,'%f\n',mapxd1(i,j));
        fprintf(fidy,'%f\n',mapyd1(i,j));
    end
end
fclose(fidx);
fclose(fidy);