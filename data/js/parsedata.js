var inputElement = document.getElementById("input");
inputElement.addEventListener("change",
    handleFiles, false);


function handleFiles(e) {
    var file = e.target.files[0];
    var reader = new FileReader();
    reader.addEventListener("load",
        processimage, false);
    reader.readAsArrayBuffer(file);
}

function processimage(e) {
    var buffer = e.target.result;
    var bitmap = getBMP(buffer);
    var unit8Array = decodePixelToHEX(bitmap);
    console.debug(unit8Array);
}

function getBMP(buffer) {
    var datav = new DataView(buffer);
    var bitmap = {};

    bitmap.fileheader = {};
    bitmap.fileheader.bfType = datav.getUint16(0, true);
    bitmap.fileheader.bfSize = datav.getUint32(2, true);
    bitmap.fileheader.bfReserved1 = datav.getUint16(6, true);
    bitmap.fileheader.bfReserved2 = datav.getUint16(8, true);
    bitmap.fileheader.bfOffBits = datav.getUint32(10, true);

    bitmap.infoheader = {};
    bitmap.infoheader.biSize = datav.getUint32(14, true);
    bitmap.infoheader.biWidth = datav.getUint32(18, true);
    bitmap.infoheader.biHeight = datav.getUint32(22, true);
    bitmap.infoheader.biPlanes = datav.getUint16(26, true);
    bitmap.infoheader.biBitCount = datav.getUint16(28, true);
    bitmap.infoheader.biCompression = datav.getUint32(30, true);
    bitmap.infoheader.biSizeImage = datav.getUint32(34, true);
    bitmap.infoheader.biXPelsPerMeter = datav.getUint32(38, true);
    bitmap.infoheader.biYPelsPerMeter = datav.getUint32(42, true);
    bitmap.infoheader.biClrUsed = datav.getUint32(46, true);
    bitmap.infoheader.biClrImportant = datav.getUint32(50, true);

    var start = bitmap.fileheader.bfOffBits;  
    
    bitmap.stride = Math.floor((bitmap.infoheader.biBitCount *bitmap.infoheader.biWidth + 31) / 32) * 4;
    bitmap.pixels = new Uint8Array(buffer, start);
    console.debug(bitmap.pixels);
    return bitmap;
}


function rgbToHex(r, g, b) {
    return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
  }

  //https://stackoverflow.com/questions/5623838/rgb-to-hex-and-hex-to-rgb
  function hexToRgb(hex) {
    // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
    var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
    hex = hex.replace(shorthandRegex, function(m, r, g, b) {
      return r + r + g + g + b + b;
    });
  
  }

  function decodePixelToHEX(bitmap){
      
      for (var i = 0; i < bitmap.infoheader.biSizeImage; i+=3) { 
        let g = bitmap.pixels[i];  //b
        let r= bitmap.pixels[i+1]; //r
        let b = bitmap.pixels[i+2]; //g
        let hexcolor = rgbToHex(r,b,g);
        console.debug(hexcolor);
       }
  }