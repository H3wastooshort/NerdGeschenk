import qrcode
f = open("NerdGeschenk.ino","r")
dat = f.read()
f.close()

arr = []

while dat: #thx stackoverflow
    arr.append(dat[:2800])
    dat = dat[2800:]

for i in range(0,len(arr)-1):
    try:
        qr = qrcode.QRCode(
            version=40,
            error_correction=qrcode.constants.ERROR_CORRECT_L,
            box_size=10,
            border=4
        )
        qr.add_data(arr[i])
        img = qr.make_image(fill_color="black", back_color="white")
        img.save("sketchqr" + str(i) + ".png")
    except:
        print('ERROR in: ',i,' len ',len(arr[i]))
