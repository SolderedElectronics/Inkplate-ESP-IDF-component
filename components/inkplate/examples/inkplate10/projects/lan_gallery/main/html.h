#ifndef HTML_H
#define HTML_H

// Upload page served at "/". The browser resizes/re-encodes the picked
// image to a JPEG that fits the Inkplate 10 panel resolution before
// POSTing it to "/upload", so uploads stay small and are always JPEG on
// the wire (this lets main/webserver.cpp locate the picture without a
// full multipart parser, see the note in main/main.cpp).
const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Inkplate LAN Gallery</title>
<style>
body{font-family:"Segoe UI",sans-serif;background:#f3f3f3;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1rem;box-sizing:border-box}
.container{background:#fff;padding:2rem 3rem;border-radius:12px;box-shadow:0 3px 12px rgba(0,0,0,.15);text-align:center;max-width:420px}
input[type=file]{margin:20px 0}
input[type=submit]{background:#0078d7;border:none;color:#fff;padding:10px 22px;border-radius:6px;cursor:pointer;font-size:1rem}
input[type=submit]:hover{background:#005fa3}
#status{margin-top:15px;font-size:.9rem;color:#555}
footer{margin-top:20px;font-size:.8rem;color:#999}
</style>
</head>
<body>
<div class="container">
<h2>Inkplate LAN Gallery</h2>
<p>Upload an image to the shared gallery. It will be added to the SD card and shown on the e-paper display.</p>
<form id="uploadForm">
<input type="file" id="fileInput" accept="image/*"><br>
<input type="submit" value="Upload">
</form>
<p id="status"></p>
</div>
<footer>Images rotate automatically on the Inkplate display.</footer>
<script>
const f=document.getElementById('uploadForm'),i=document.getElementById('fileInput'),s=document.getElementById('status');
f.addEventListener('submit',async e=>{
 e.preventDefault();const file=i.files[0];if(!file)return;
 s.textContent='Processing image...';
 const img=new Image();img.src=URL.createObjectURL(file);await img.decode();
 const maxW=1200,maxH=825;let w=img.width,h=img.height;
 if(w>maxW||h>maxH){const r=Math.min(maxW/w,maxH/h);w=Math.floor(w*r);h=Math.floor(h*r);s.textContent='Resizing image...';}
 const c=document.createElement('canvas');c.width=w;c.height=h;const x=c.getContext('2d');x.drawImage(img,0,0,w,h);
 const blob=await new Promise(r=>c.toBlob(r,'image/jpeg',0.85));
 const name=file.name.replace(/\.[^.]+$/,'.jpg');
 const up=new File([blob],name,{type:'image/jpeg'});
 URL.revokeObjectURL(img.src);
 const fd=new FormData();fd.append('file',up,name);s.textContent='Uploading...';
 try{const r=await fetch('/upload',{method:'POST',body:fd});const t=await r.text();s.textContent=t.includes('OK')?'Upload complete!':t;}catch(err){s.textContent='Error: '+err;}
});
</script>
</body>
</html>
)rawliteral";

#endif
