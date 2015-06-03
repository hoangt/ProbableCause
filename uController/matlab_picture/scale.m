img = imread('org_input.bmp');
img2 = reshape(img,1,30800);
img3 = dec2hex(img2);
filename = 'scale.txt';
fid = fopen(filename, 'w');
fprintf(fid,'@3500\n');
i=1
while i<=(200*154)
    fprintf(fid,'%c%c ',img3(i),img3(i,2));
    i=i+1
    if mod(i,16) == 1
        fprintf(fid, '\n');
    end
end
fprintf(fid,'\nq');
fclose(fid);
