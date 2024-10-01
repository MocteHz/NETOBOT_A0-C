
const apptst= {
    data() {
      return {
          Titulo: 'Neto-Beot',
          showModal: false,
          showHome: true,
          showPatterns: false,
          showCloud: false,
          showConfig: false,
          //Valores de config
          sliderVelocidad: 50,
          sliderBrillo: 50,
          sliderTempColor: 50,
          selectedColor:0,

          //Valor para thetarho
          svgString: '',
          outputSize: 300,
          inputFileName: '',
          memory: [ ],
          totalMemory: 0, 
          StatusMemory: 0, 
          UsedMemory: 0,
          websocket: [],

          displayList: false,
          fileList: [],
          allList: [],
          file: null,
          uploadUrl: "/upload",
          showModal: false,
          author: '',
          reversible: false,
          origin: false,
          filestoshow:[],
          
          
      }
    },
    mounted() {
      this.initWebSockets();
    },
    methods: {
        initWebSockets() {
            //ask for a nickname
            //connect to Sockets Bay
            this.websocket = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
            console.log("Iniciando WebSockets");
            //
            this.websocket.onopen    = this.onSocketOpen;
            this.websocket.onmessage = this.onSocketMessage;
            this.websocket.onerror   = this.onSocketError;
            console.log("Se iniciaron");
          },
        onSocketOpen(evt) {
            this.connection_ready = true;
            this.websocket.send("ConnectionStatus");
            this.websocket.send("GetJsonString");
            this.websocket.send("GetSDStatus");
        },
        onSocketMessage(evt) {
            console.log(evt.data);             
            var Recibed = JSON.parse(evt.data); 
            console.log(Recibed);
            if (Recibed.storageType != undefined){
                this.totalMemory = this.convertBytes(Recibed.totalSpace);
                this.StatusMemory = Recibed.initStatus;
                this.UsedMemory = this.convertBytes(Recibed.usedSpace);
            }
            if (Recibed.type === "allFilesPL" || Recibed.type === "allLikedPL" || Recibed.type === "playlist") {
                this.displayList = true;
                this.allList = Recibed.payload;
                // Process the payload and display the content
            }
            if (Recibed && Recibed.type === "upload-progress") {
                const percentComplete = Math.round(
                  (Recibed.loaded / Recibed.total) * 100
                );
                this.updateProgressBar(percentComplete);
            } 
            if (Recibed && Recibed.type === "upload-complete") {
                this.uploadComplete();
            }
            if (Recibed && Recibed.type === "allFilesPL") {
                this.filestoshow = Recibed.payload.map(item => {
                    const [filename, date, owner, size, locked, shared] = item.split(',');
                    return {
                      filename,
                      date,
                      owner,
                      size: parseInt(size, 10),
                      locked: parseInt(locked, 10),
                      shared: parseInt(shared, 10)
                    };
                  });
                  console.log("Archivo Parseado");
                  console.log(this.filestoshow);
            }
          
        },
        openFileDialog() {
          this.$refs.fileInput.click();
        },
        async uploadFiles() {
          try {
              //console.log(this.sendUploadRequest)
              // Upload the generated PNG file
              const pngBlob = await this.exportPNG1();
              const pngFileName = this.inputFileName + '.png';
              this.uploadFile1(pngFileName, pngBlob);
      
              // Upload the other file
              if (this.file) {
              this.uploadFile1(this.file.name, this.file);
              }
          } catch (error) {
              console.error('Error uploading files:', error);
          }
        },
        uploadFile1(filename, file) {
            console.log("Upload File:", filename);
            const reader = new FileReader();
            reader.onload = () => {
                const buffer = reader.result;
                const filesize = file.size;
                this.sendUploadRequest(filename, filesize, buffer);
            };
            reader.readAsArrayBuffer(file);
        },
        uploadFile() {
            console.log("Upload File");
            if (this.file) {
                const reader = new FileReader();
                reader.onload = () => {
                const buffer = reader.result;
                const filename = this.file.name;
                const filesize = this.file.size;
                this.sendUploadRequest(filename, filesize, buffer);
                };
                reader.readAsArrayBuffer(this.file);
            }
        },
        sendUploadRequest(filename, filesize, buffer) {
            const fileExtension = filename.split('.').pop();
            const currentDate = new Date();
            const formattedDate = new Intl.DateTimeFormat('en', {
                year: 'numeric',
                month: '2-digit',
                day: '2-digit'
            }).format(currentDate);
            const message = {
                type: "upload-request",
                filename: filename,
                date: formattedDate, // Assuming you have a 'date' data property
                author: this.author, // Assuming you have an 'author' data property
                length: filesize,
                reversible: this.reversible, // Assuming you have a 'reversible' data property
                origin: this.origin, // Assuming you have an 'origin' data property
                filetype: fileExtension, // Adding the file extension
            };
            console.log(message);
            this.websocket.send(JSON.stringify(message));
            const chunkSize = 4096;
            let offset = 0;
            while (offset < buffer.byteLength) {
                const slice = buffer.slice(offset, offset + chunkSize);
                this.websocket.send(slice);
                offset += slice.byteLength;
            }
        },
        sendUploadRequestf() {
            const uploadData = {
                Type: 'UploadRequest',
                FileName: 'your_file_name', // Replace with the actual file name
                Date: new Date().toISOString(),
                Author: this.author,
                Length: 0, // Replace with the actual file length in bytes
                Reversible: this.reversible,
                Origin: this.origin,
            };
        },
        updateProgressBar(percentComplete) {
            const progressBar = document.getElementById("progress-bar");
            progressBar.value = percentComplete;
        },
        uploadComplete() {
            const progressBar = document.getElementById("progress-bar");
            progressBar.value = 100;
        },
        onFileChange(event) {
            this.file = event.target.files[0];
            this.showModal = true;
            this.processThetaRhoFile(event);
        },
        convertBytes(bytes) {
            const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
            if (bytes === 0) return '0 Bytes';
            const i = Math.floor(Math.log(bytes) / Math.log(1024));
            return parseFloat((bytes / Math.pow(1024, i)).toFixed(2)) + ' ' + sizes[i];
        },
        updateSliderVel() {
            console.log('Slider de velocidad cambió a:', this.sliderVelocidad);
            const message = {
                SetSpeed: this.sliderVelocidad
            };
            console.log(message);
            this.websocket.send(JSON.stringify(message));
            // Call your function or send the value to your server here
        },
        updateSliderBrillo() {
            console.log('Slider de Brillo cambió a:', this.sliderBrillo);
            const message = {
                SetBrillo: this.sliderBrillo
            };
            console.log(message);
            this.websocket.send(JSON.stringify(message));
        },
        updateColor(){
            console.log('Color Seleccionado es:', this.selectedColor);
            const message = {
                ColorSelected: this.selectedColor
            };
            console.log(message);
            this.websocket.send(JSON.stringify(message));
        },
        listAllFiles() {
            this.websocket.send("NameAllFilesPL");
        },
        listLiked() {
            this.websocket.send("NameAllLikedPL");
        },
        listPlaylists() {
            this.websocket.send("NameAllPlaylists");
        },
        updateProgressBar(percentComplete) {
            const progressBar = document.getElementById("progress-bar");
            progressBar.value = percentComplete;
        },
        uploadComplete() {
            const progressBar = document.getElementById("progress-bar");
            progressBar.value = 100;
        },
        processThetaRhoFile(event) {
            const file = event.target.files[0];
            const reader = new FileReader();
            reader.onload = () => {
            const fileContent = reader.result;
            const lines = fileContent.split('\n');
            let pathData = 'M';
            const coordinates = [];

            // Store the input file's name without the extension
            this.inputFileName = file.name.replace(/\.[^/.]+$/, '');


            // Helper function to round a number to a specified precision
            const roundToPrecision = (number, precision) => {
                const factor = Math.pow(10, precision);
                return Math.round(number * factor) / factor;
            };

            // Determine the decimal precision in the input Theta Rho file
            const precision = Math.max(
                ...lines.map((line) => {
                const match = line.match(/\.(\d+)/);
                return match ? match[1].length : 0;
                })
            );

            lines.forEach((line) => {
                const [theta, rho] = line.split(' ').map(parseFloat);
                if (isNaN(theta) || isNaN(rho)) return;

                // Convert polar coordinates to Cartesian coordinates
                const x = rho * Math.cos(theta);
                const y = rho * Math.sin(theta);

                coordinates.push({ x, y });
            });

            // Normalize the coordinates to fit within the 200x200 px SVG
            const minX = Math.min(...coordinates.map((c) => c.x));
            const maxX = Math.max(...coordinates.map((c) => c.x));
            const minY = Math.min(...coordinates.map((c) => c.y));
            const maxY = Math.max(...coordinates.map((c) => c.y));
            
            const width = maxX - minX;
            const height = maxY - minY;

            const scaleX = this.outputSize / width;
            const scaleY = this.outputSize / height;

            coordinates.forEach((c) => {
                const normalizedX = roundToPrecision((c.x - minX) * scaleX, precision);
                const normalizedY = roundToPrecision((c.y - minY) * scaleY, precision);
                pathData += `${normalizedX},${normalizedY} `;
            });

            // Set the SVG string
            this.svgString = `
                <svg viewBox="0 0 ${this.outputSize} ${this.outputSize}" xmlns="http://www.w3.org/2000/svg" width="${this.outputSize}" height="${this.outputSize}">
                <path d="${pathData}" stroke="grey" fill="none" />
                </svg>
            `;
            };
            reader.readAsText(file);
        },
        exportSVG() {
            const svgBlob = new Blob([this.svgString], { type: 'image/svg+xml;charset=utf-8' });
            const svgURL = URL.createObjectURL(svgBlob);
            const downloadLink = document.createElement('a');
            downloadLink.href = svgURL;
            downloadLink.download = this.inputFileName + '.svg';
            document.body.appendChild(downloadLink);
            downloadLink.click();
            document.body.removeChild(downloadLink);
        },
        exportPNG() {
            const svgElement = new DOMParser().parseFromString(this.svgString, 'image/svg+xml').firstChild;
            const canvas = document.createElement('canvas');
            canvas.width = 200;
            canvas.height = 200;
            const ctx = canvas.getContext('2d');

            // Convert the SVG element to a data URL
            const svgData = new XMLSerializer().serializeToString(svgElement);
            const svgDataURL = 'data:image/svg+xml;base64,' + btoa(svgData);

            const image = new Image();
            image.onload = () => {
            ctx.drawImage(image, 0, 0, 200, 200);

            // Create a PNG data URL from the canvas
            const pngDataURL = canvas.toDataURL('image/png');

            // Trigger the download of the PNG file
            const downloadLink = document.createElement('a');
            downloadLink.href = pngDataURL;
            downloadLink.download = this.inputFileName + '.png';
            document.body.appendChild(downloadLink);
            downloadLink.click();
            document.body.removeChild(downloadLink);
            };
            image.src = svgDataURL;
        },
        exportPNG1() {
            return new Promise((resolve, reject) => {
                const svgElement = new DOMParser().parseFromString(this.svgString, 'image/svg+xml').firstChild;
                const canvas = document.createElement('canvas');
                canvas.width = 200;
                canvas.height = 200;
                const ctx = canvas.getContext('2d');
            
                // Convert the SVG element to a data URL
                const svgData = new XMLSerializer().serializeToString(svgElement);
                const svgDataURL = 'data:image/svg+xml;base64,' + btoa(svgData);
            
                const image = new Image();
                image.onload = () => {
                ctx.drawImage(image, 0, 0, 200, 200);
            
                // Create a PNG data URL from the canvas
                const pngDataURL = canvas.toDataURL('image/png');
            
                // Convert the PNG data URL to a Blob
                fetch(pngDataURL)
                    .then((res) => res.blob())
                    .then((blob) => {
                    resolve(blob);
                    })
                    .catch((error) => {
                    reject(error);
                    });
                };
                image.onerror = (error) => {
                reject(error);
                };
                image.src = svgDataURL;
            });
        },
        getFilenameWithoutExtension(item) {
            const [filename] = item.split(",");
            console.log( [filename]);
            return filename.replace(/\.[^/.]+$/, "");
        },
        getImagePath(item) {
            const [filename] = item.split(",");
            const filenameWithoutExtension = filename.replace(/\.[^/.]+$/, "");
            return `/Images/${filenameWithoutExtension}.png`;
        },
        getAuthor(item) {
            const [, , author] = item.split(",");
            return author;
        },
        getDate(item) {
            const [, date] = item.split(",");
            return date;
        },
    }
}
  
Vue.createApp(apptst).mount('#app');


 