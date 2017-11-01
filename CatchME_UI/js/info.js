var fwForm = getE("ota"),
	firmware = getE("update"),
	versionCell = getE("version"),
	freememory = getE("freememory"),
	flashsize = getE("flashsize"),
	bootversion = getE("bootversion"),
	chipid = getE("chipid"),
	deauthpackets = getE("deauthpackets"),
	beaconpackets = getE("beaconpackets"),
	flashid = getE("flashid"),
	ipaddress = getE("ipaddress"),
	gateway = getE("gateway"),
	sdk = getE("sdk"),
	bootmode = getE("bootmode"),
	uptime = getE("uptime"),
	fm, fz, bm, bv, fi, ci, sk, dp, ut, bp, ip, gw;

function getData() {
	getResponse("sysinfo.json", function (responseText) {
		notify();
		var res = JSON.parse(responseText);
		fm = res.freememory; //res.freememory.replace(/(.)(?=(\d{3})+$)/g, '$1,');
		fz = res.flashchipsize;
		bm = res.bootmode;
		bv = res.bootversion;
		fi = res.flashchipid;
		ci = res.chipid;
		sk = res.sdkversion;
		dp = res.deauthpackets;
		bp = res.beaconpackets;
		ut = res.uptime.slice(0, -3);
		ip = res.ipaddress;
		gw = res.gateway;

		freememory.innerHTML = "<b>" + bytesToSize(fm) + "</b> / " + bytesToSize(64000);
		flashsize.innerHTML = bytesToSize(fz);
		bootmode.innerHTML = bm;
		bootversion.innerHTML = bv;
		flashid.innerHTML = fi;
		chipid.innerHTML = ci;
		deauthpackets.innerHTML = dp;
		beaconpackets.innerHTML = bp;
		sdk.innerHTML = "v." + sk;
		uptime.innerHTML = new Date(ut * 1000).toISOString().substr(11, 8);
		ipaddress.innerHTML = ip;
		gateway.innerHTML = gw;
		versionCell.innerHTML = version;
		fadeIn();
	}, function () {
		notify('Failed to load sysinfo.json');
		fadeIn();
	});
}

function bytesToSize(bytes) {
	var sizes = [' bytes', 'KB', 'MB', 'GB', 'TB'];
	if (bytes == 0) return '0 Byte';
	var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
	return Math.round(bytes / Math.pow(1024, i), 2) + '' + sizes[i];
};

function checkUpdate() {
	window.open("#linkforcheckupdate" + version + "&sdk=" + sk + "&freememory=" + fm + "&flashsize=" + fz + "&bootmode=" + bm + "&bootversion=" + bv + "&flashid=" + fi + "&chipid=" + ci + "&deauthpackets=" + dp + "&beaconpackets=" + bp + "&ipaddress=" + ip + "&gateway=" + gw, '_blank');
}

function updateFirmware() {
	if (confirm("Are you sure want to update Firmware?") == true) {
		var url = "update",
			method = "POST",
			fw = fwForm,
			fwBin = firmware.value,
			data = new FormData(fw);

		if (fwBin == "" || fwBin == null) {
			notify("ERROR: Firmware should not be empty!", 3000);
		} else {
			var xmlhttp = new XMLHttpRequest();
			xmlhttp.onreadystatechange = function () {
				if (xmlhttp.readyState == 4) {
					if (xmlhttp.status == 200) {
						if(xmlhttp.responseText == "ok") {
							notify("Update Firmware success!, CatchMe will be restart!", 5000);
							setTimeout(function () { window.location = "/" }, 30000);
						} else {
							notify("ERROR: Failed to update Firmware!", 3000);
							setTimeout(function(){window.location.reload()},4000);
						}
					} else {
						notify("ERROR: Failed to update Firmware!", 3000);
						setTimeout(function(){window.location.reload()},4000);
					}
				}
			};

			xmlhttp.open(method, url, true);
			xmlhttp.send(data);
		}
	}
}

var input = document.getElementById("update"),
	label = input.nextElementSibling,
	uploadSvg = '<i><svg viewBox="0 0 24 24"><path d="M21,19A2,2 0 0,1 19,21H5A2,2 0 0,1 3,19V5A2,2 0 0,1 5,3H19C20.11,3 21,3.9 21,5V19M13,18V9.5L16.5,13L17.92,11.58L12,5.66L6.08,11.58L7.5,13L11,9.5V18H13Z"/></svg></i>';
label.innerHTML = uploadSvg + "SELECT FIRMWARE FILE .BIN";
labelVal = label.innerHTML;


input.addEventListener('change', function (e) {
	var fileName = e.target.value.split('\\').pop();
	if (fileName)
		label.innerHTML = uploadSvg + fileName;
	else
		label.innerHTML = labelVal;
});

getData();
infoInterval = setInterval(getData, 3000);

