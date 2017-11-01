var ssid = getE('ssid'),
	ssidHidden = getE('ssidHidden'),
	password = getE('password'),
	apChannel = getE('apChannel'),
	macAp = getE('macAp'),
	randMacAp = getE('randMacAp'),
	apScanHidden = getE('apScanHidden'),
	scanTime = getE('scanTime'),
	timeout = getE('timeout'),
	deauthReason = getE('deauthReason'),
	packetRate = getE('packetRate'),
	clientNames = getE('clientNames'),
	useLed = getE('useLed'),
	useAlertLed = getE('useAlertLed'),
	channelHop = getE('channelHop'),
	multiAPs = getE('multiAPs'),
	multiAttacks = getE('multiAttacks'),
	cMac = getE('cMac'),
	cName = getE('cName'),
	macInterval = getE('macInterval'),
	beaconInterval = getE('beaconInterval'),
	ledPin = getE('ledPin'),
	alertLedPin = getE('alertLedPin'),
	ledContainer = getE('ledContainer'),
	alertLedContainer = getE('alertLedContainer'),
	macContainer = getE('macContainer'),
	cacheContainer = getE('cacheContainer'),
	serverCache = getE('serverCache'),
	wifiClient = getE('wifiClient'),
	ssidClient = getE('ssidClient'),
	passwordClient = getE('passwordClient'),
	hostname = getE('hostname'),
	clientContainer = getE('clientContainer'),
	cache = getE('cache'),
	res = '',
	checkboxChanges,
	inputChanges;

/* Add listeners to checkboxes */
useLed.addEventListener("change", switchLED, false);
useAlertLed.addEventListener("change", switchAlertLED, false);
cache.addEventListener("change", switchCache, false);
randMacAp.addEventListener("change", switchMAC, false);
wifiClient.addEventListener("change", switchClient, false);

function getData() {
	getResponse("settings.json", function (responseText) {
		try {
			res = JSON.parse(responseText);
		} catch (e) {
			fadeIn();
			notify("ERROR: Reset the settings.  (E17)");
			return;
		}
		ssid.value = res.ssid;
		ssidHidden.checked = res.ssidHidden;
		password.value = res.password;
		apChannel.value = res.apChannel;
		macAp.value = res.macAp;
		randMacAp.checked = res.randMacAp;
		apScanHidden.checked = res.apScanHidden;
		scanTime.value = res.clientScanTime;
		timeout.value = res.attackTimeout;
		deauthReason.value = res.deauthReason;
		packetRate.value = res.attackPacketRate;
		useLed.checked = res.useLed;
		useAlertLed.checked = res.useAlertLed;
		/*channelHop.checked = res.channelHop;*/
		multiAPs.checked = res.multiAPs;
		multiAttacks.checked = res.multiAttacks;
		macInterval.value = res.macInterval;
		beaconInterval.checked = res.beaconInterval;
		ledPin.value = res.ledPin;
		alertLedPin.value = res.alertLedPin;
		cache.checked = res.cache;
		serverCache.value = res.serverCache;
		wifiClient.checked = res.wifiClient;
		ssidClient.value = res.ssidClient;
		passwordClient.value = res.passwordClient;
		hostname.value = res.hostname;
		switchLED();
		switchAlertLED();
		switchMAC();
		switchCache();
		switchClient();
		fadeIn();
	}, function () {
		notify("ERROR: Failed to load 'settings.json'");
		fadeIn();
	});
}

function saveSettings() {
	indicate();
	showLoading();
	var url = "settingsSave.json";
	url += "?ssid=" + ssid.value;
	url += "&ssidHidden=" + ssidHidden.checked;
	url += "&password=" + password.value;
	url += "&apChannel=" + apChannel.value;
	url += "&macAp=" + macAp.value;
	url += "&randMacAp=" + randMacAp.checked;
	url += "&apScanHidden=" + apScanHidden.checked;
	url += "&scanTime=" + scanTime.value;
	url += "&timeout=" + timeout.value;
	url += "&deauthReason=" + deauthReason.value;
	url += "&packetRate=" + packetRate.value;
	url += "&useLed=" + useLed.checked;
	url += "&useAlertLed=" + useAlertLed.checked;
	/*url += "&channelHop=" + channelHop.checked;*/
	url += "&multiAPs=" + multiAPs.checked;
	url += "&multiAttacks=" + multiAttacks.checked;
	url += "&macInterval=" + macInterval.value;
	url += "&beaconInterval=" + beaconInterval.checked;
	url += "&ledPin=" + ledPin.value;
	url += "&alertLedPin=" + alertLedPin.value;
	url += "&cache=" + cache.checked;
	url += "&serverCache=" + serverCache.value;
	url += "&wifiClient=" + wifiClient.checked;
	url += "&ssidClient=" + ssidClient.value;
	url += "&passwordClient=" + passwordClient.value;
	url += "&hostname=" + hostname.value;

	getResponse(url, function (responseText) {
		if (responseText == "true") {
			getData();
			indicate(true);
			inputChanges = false;
			checkboxChanges = false;
		} else {
			indicate(false);
			notify("ERROR: Failed to save settings!");
		}
	}, function () {
		getResponse("settings.json", function (responseText) {
			getData();
			indicate(true);
			inputChanges = false;
			checkboxChanges = false;
		}, function () {
			indicate(false);
			notify("ERROR: Failed to load settings!");
		});
	});
}

function resetSettings() {
	if (confirm("Reset all to default settings?") == true) {
		showLoading();
		getResponse("settingsReset.json", function (responseText) {
			if (responseText == "true") {
				getData();
				indicate(true);
				restart(true);
				setTimeout(function () { window.location = "/" }, 3000)
			} else {
				notify("ERROR: Failed to reset settings!");
				indicate(false);
			}
		}, function () {
			notify("ERROR: Failed to reset settings!");
			indicate(false);
		});
	}
	inputChanges = false;
	checkboxChanges = false;
}

function switchLED() {
	if (useLed.checked)
		ledContainer.classList.remove("disabled");
	else
		ledContainer.classList.add("disabled");
}

function switchAlertLED() {
	if (useAlertLed.checked)
		alertLedContainer.classList.remove("disabled");
	else
		alertLedContainer.classList.add("disabled");
}

function switchCache() {
	if (cache.checked)
		cacheContainer.classList.remove("disabled");
	else
		cacheContainer.classList.add("disabled");
}

function switchMAC() {
	if (randMacAp.checked)
		macContainer.classList.add("disabled");
	else
		macContainer.classList.remove("disabled");
}

function switchClient() {
	if (wifiClient.checked)
		clientContainer.classList.remove("disabled");
	else
		clientContainer.classList.add("disabled");
}

/* Detect form changes and display popup if not saved */
var form = document.getElementById("settings");
form.addEventListener("input", function () {
	inputChanges = true;
});

form.addEventListener("change", function () {
	checkboxChanges = true;
}, false);

window.addEventListener("beforeunload", function (e) {
	if (inputChanges || checkboxChanges) {
		var confirmationMessage = 'All changes will be lost!';
		(e || window.event).returnValue = confirmationMessage;
		return confirmationMessage;
	}
});

getData();
