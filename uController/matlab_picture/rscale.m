filename = 'scale.txt';
res2=char.empty(154,0);
res = textread(filename, '%s');
size(res)

for i=2:200*154+1
    s=char(res(i));
    res2(2*(i-1)-1)=s(1);
    res2(2*(i-1))=s(2);
end
res3=uint8(zeros(154,200));

for i=1:200*154
    res3(i)=uint8(hex2dec(res2(2*i-1)))*16+uint8(hex2dec(res2(2*i)));
    %res3(i)
    i=i+1;
end

imwrite(res3,'output.bmp','bmp');