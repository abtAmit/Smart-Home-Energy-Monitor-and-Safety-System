// --- HTML for the Login Page ---
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Home Login</title>
  <style>
    body { margin: 0; padding: 0; font-family: Arial, sans-serif; height: 100vh; display: flex; justify-content: center; align-items: center; background: linear-gradient(135deg, #6377f1, #24d8e3); color: white; text-align: center; }
    .container { width: 100%; max-width: 320px; padding: 20px; }
    .logo svg { width: 80px; height: 80px; stroke: white; stroke-width: 2; fill: none; margin-bottom: 15px; }
    h1 { font-size: 24px; margin-bottom: 30px; } h1 span { font-weight: bold; }
    .input-field { background: rgba(255, 255, 255, 0.2); border: none; border-radius: 25px; padding: 12px 15px; margin: 10px 0; width: 100%; box-sizing: border-box; color: white; font-size: 14px; outline: none; }
    .input-field::placeholder { color: #eee; }
    .btn { margin-top: 15px; background: white; color: #6366f1; border: none; border-radius: 25px; padding: 12px; width: 100%; font-size: 16px; font-weight: bold; cursor: pointer; transition: 0.3s; }
    .btn:hover { background: #ddd; }
    .error { color: #fff; background: rgba(255,0,0,0.3); padding: 5px; border-radius: 5px; margin-top: 10px; }
  </style>
</head>
<body>
  <div class="container">
    <div class="logo">
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M3 9.5L12 3l9 6.5V21a1 1 0 0 1-1 1h-6v-6h-4v6H4a1 1 0 0 1-1-1V9.5z"/></svg>
    </div>
    <h1>SMART <span>HOME</span></h1>
    <form method="POST" action="/login">
      <input type="text" class="input-field" placeholder="Username or Email" required name="username">
      <input type="password" class="input-field" placeholder="Password" required name="password">
      <button type="submit" class="btn">Login</button>
      <p id="errorMsg" class="error" style="display:none;"></p>
    </form>
  </div>
  <script>
    const params = new URLSearchParams(window.location.search);
    if (params.has('error')) {
      if (params.get('error') == 'full') {
        document.getElementById('errorMsg').innerText = 'Server is full! (Max 3 users).';
      } else {
        document.getElementById('errorMsg').innerText = 'Invalid username or password!';
      }
      document.getElementById('errorMsg').style.display = 'block';
    }
  </script>
</body>
</html>
)rawliteral";





// --- HTML page stored in PROGMEM (Flash Memory) ---



const char HTML_PROGMEM[] PROGMEM = R"RAW_HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Home UI</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <script src="https://unpkg.com/lucide@latest"></script>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link rel="icon" href="https://cdn-icons-png.flaticon.com/512/1427/1427247.png" type="image/x-icon">
  <link rel="apple-touch-icon" href="https://cdn-icons-png.flaticon.com/512/1427/1427247.png">
    <style>
        body { font-family: 'Inter', sans-serif; background-color: #f4f6f9; -webkit-tap-highlight-color: transparent; padding-bottom: 80px; }
        .view { display: none; }
        .view.active { display: block; }
        .room-btn { position: relative; background: white; border-radius: 1rem; padding: 25px 10px; border: none; cursor: pointer; transition: all 0.3s ease; box-shadow: 0 2px 6px rgba(0,0,0,0.08); display: flex; flex-direction: column; align-items: center; justify-content: center; font-size: 14px; color: #666; }
        .room-btn i { font-size: 30px; margin-bottom: 8px; color: #1d6eb9; }
        .room-options-btn { position: absolute; top: 5px; right: 5px; background: transparent; border: none; border-radius: 50%; width: 28px; height: 28px; cursor: pointer; display: flex; align-items: center; justify-content: center; z-index: 5; font-size: 20px; color: #888; line-height: 1; }
        .room-options-btn:hover { background-color: #f0f0f0; }
        .room-options-btn::after { content: '\2807'; }
        .room-options-menu { display: none; position: absolute; top: 40px; right: 8px; background: white; border-radius: 8px; box-shadow: 0 4px 12px rgba(0,0,0,0.15); z-index: 10; overflow: hidden; width: 120px; }
        .room-options-menu.show { display: block; }
        .room-options-menu button { display: block; width: 100%; padding: 10px 15px; border: none; background: none; text-align: left; cursor: pointer; font-size: 14px; }
        .room-options-menu button:hover { background-color: #f5f5f5; }
        .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.5); display: none; justify-content: center; align-items: center; z-index: 1000; }
        .modal-content { background: white; padding: 25px; border-radius: 16px; width: 85vw; max-width: 350px; box-shadow: 0 5px 15px rgba(0,0,0,0.2); position: relative; text-align: center; }
        .device-card { transition: all 0.3s ease; position: relative; overflow: hidden; box-shadow: 0 4px 12px rgba(0,0,0,0.08); }
        .device-card.off { background-color: #ffffff; color: #1f2937; }
        .device-card.on { background: linear-gradient(135deg, #6377f1, #24d8e3); color: #ffffff; }
        input[type="range"].vertical-slider { -webkit-appearance: none; appearance: none; width: 8px; height: 100%; border-radius: 5px; cursor: pointer; writing-mode: bt-lr; -webkit-appearance: slider-vertical; transition: opacity 0.3s ease, background-color: 0.3s ease; }
        .off input[type="range"].vertical-slider { opacity: 0.7; pointer-events: none; background-color: #9ca3af; }
        .on input[type="range"].vertical-slider { opacity: 1; pointer-events: auto; background-color: rgba(255, 255, 255, 0.4); }
        input[type="range"].vertical-slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 20px; height: 20px; background: #ffffff; border-radius: 50%; }
        .icon-selection-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(40px, 1fr)); gap: 0.5rem; }
        .icon-selection-grid .icon-wrapper { border: 2px solid transparent; border-radius: 0.5rem; padding: 0.5rem; cursor: pointer; display: flex; align-items: center; justify-content: center; }
        .icon-selection-grid .icon-wrapper.selected { border-color: #4f46e5; background-color: #e0e7ff; }
        .schedule-info { font-size: 0.75rem; min-height: 2.5rem; line-height: 1.5; font-family: monospace; }
        .back-button { background: none; border: none; font-size: 24px; cursor: pointer; color: #333; }
        .power-status { display: flex; justify-content: space-around; flex-wrap: wrap; gap: 10px; margin: 20px 0; padding: 20px; background: white; border-radius: 16px; box-shadow: 0 2px 6px rgba(0,0,0,0.08); }
        .status-item { display: flex; gap: 12px; text-align: left; flex-basis: 45%; justify-content: center; }
        .status-item i { font-size: 28px; color: #3e67bf; }
        .status-item div { display: flex; flex-direction: column; }
        .status-item span { font-weight: bold; color: #333; font-size: 16px; }
        .bottom-nav { position: fixed; bottom: 0; left: 0; width: 100%; background: white; display: flex; justify-content: space-around; padding: 12px 0; border-top: 1px solid #ddd; z-index: 100; }
        .nav-btn { background: none; border: none; cursor: pointer; font-size: 20px; color: #666; transition: 0.3s; }
        .nav-btn.active { color: #3e67bf; }
    </style>
</head>
<body>
    <div id="home-view" class="view active">
        <div class="max-w-4xl mx-auto p-4">
             <h1 class="text-2xl font-bold text-gray-800 my-4 text-center">Welcome <span>Home</span></h1>
             <div class="power-status">
                <div class="status-item"><i class="fa-solid fa-bolt-lightning"></i><div><span id="voltage-display">0V</span><small>Voltage</small></div></div>
                <div class="status-item"><i class="fa-solid fa-chart-line"></i><div><span id="pf-display">0.00</span><small>Power Factor</small></div></div>
                <div class="status-item"><i class="fa-solid fa-plug-circle-bolt"></i><div><span id="wattage-display">0W</span><small>Consumption</small></div></div>
             </div>
             <div id="rooms-grid" class="grid grid-cols-2 sm:grid-cols-3 gap-4 md:gap-6"></div>
        </div>
    </div>
    <div id="room-view" class="view">
        <div class="max-w-4xl mx-auto p-4">
            <div class="flex items-center justify-between mb-4">
                <button id="back-to-home" class="back-button">&larr;</button>
                <h1 id="room-title" class="text-2xl font-bold text-gray-800 text-center flex-grow">Room</h1>
                <div class="w-8"></div>
            </div>
            <main id="device-grid" class="grid grid-cols-2 sm:grid-cols-3 gap-4 md:gap-6"></main>
        </div>
    </div>
    <div class="bottom-nav">
        <button class="nav-btn active" onclick="currentRoomId = null; showView('home-view');"> <i class="fa-solid fa-house"></i></button>
        <button class="nav-btn" onclick="window.location.href='/power'"> <i class="fa-solid fa-bolt"></i></button>
        <button class="nav-btn" onclick="window.location.href='/graph'"><i class="fa-solid fa-chart-column"></i></button>
        <button class="nav-btn" onclick="window.location.href='/settings'"> <i class="fa-solid fa-gear"></i></button>
    </div>
    <div id="room-modify-modal" class="modal-overlay">
        <div class="modal-content">
            <h2 id="room-modal-title" class="text-xl font-bold mb-1">Edit Room</h2>
            <p id="room-id-display" class="text-sm text-gray-500 mb-4"></p>
            <div class="text-left">
                 <label for="roomName" class="block text-sm font-medium text-gray-700">Room Name</label>
                <input type="text" id="roomName" class="mt-1 block w-full px-3 py-2 border border-gray-300 rounded-md shadow-sm">
            </div>
            <div class="text-left mt-4">
                <label class="block text-sm font-medium text-gray-700">Icon</label>
                <div id="room-icon-list" class="icon-selection-grid p-2 max-h-32 overflow-y-auto"></div>
            </div>
            <div class="flex justify-end space-x-2 mt-4">
                <button type="button" class="cancel-btn px-4 py-2 bg-gray-200 rounded-md">Cancel</button>
                <button type="button" id="save-room-btn" class="px-4 py-2 bg-indigo-600 text-white rounded-md">Save</button>
            </div>
        </div>
    </div>
    <div id="device-modify-modal" class="modal-overlay">
        <div class="modal-content">
            <h2 id="device-modal-title" class="text-xl font-bold mb-4">Modify Device</h2>
            <form id="modify-form">
                <div class="mb-4 text-left">
                    <label for="deviceName" class="block text-sm font-medium text-gray-700">Name</label>
                    <input type="text" id="deviceName" class="mt-1 block w-full px-3 py-2 border border-gray-300 rounded-md" required>
                </div>
                <div class="mb-4 text-left">
                    <label for="devicePin" class="block text-sm font-medium text-gray-700">GPIO Pin</label>
                    <input type="number" id="devicePin" class="mt-1 block w-full px-3 py-2 border border-gray-300 rounded-md" required>
                </div>
                <div class="mb-4 flex items-center">
                    <input type="checkbox" id="hasSlider" class="h-4 w-4 text-indigo-600 border-gray-300 rounded">
                    <label for="hasSlider" class="ml-2 block text-sm text-gray-900">Enable Dimmer/Slider</label>
                </div>
                 <div class="mb-4 text-left">
                    <label class="block text-sm font-medium text-gray-700 mb-2">Icon</label>
                     <div id="device-icon-selection" class="icon-selection-grid max-h-32 overflow-y-auto p-2 bg-gray-50 rounded-lg"></div>
                </div>
                <div class="flex justify-end space-x-2">
                    <button type="button" class="cancel-btn px-4 py-2 bg-gray-200 rounded-md">Cancel</button>
                    <button type="submit" class="px-4 py-2 bg-indigo-600 text-white rounded-md">Save</button>
                </div>
            </form>
        </div>
    </div>
    <div id="schedule-modal" class="modal-overlay">
        <div class="modal-content">
            <h2 id="schedule-modal-title" class="text-xl font-bold mb-4">Set Schedule</h2>
            <form id="schedule-form">
                 <div class="space-y-4">
                    <div class="flex items-center space-x-4"><input type="checkbox" id="scheduleOnEnabled" class="h-4 w-4"><label for="scheduleOnTime" class="w-12">ON at</label><input type="time" id="scheduleOnTime" class="block w-full border border-gray-300 rounded-md"></div>
                    <div class="flex items-center space-x-4"><input type="checkbox" id="scheduleOffEnabled" class="h-4 w-4"><label for="scheduleOffTime" class="w-12">OFF at</label><input type="time" id="scheduleOffTime" class="block w-full border border-gray-300 rounded-md"></div>
                </div>
                 <div class="flex justify-between items-center mt-6">
                    <button type="button" id="clear-schedule" class="px-4 py-2 text-sm text-red-600 hover:bg-red-50 rounded-md">Clear</button>
                    <div class="space-x-2"> <button type="button" class="cancel-btn px-4 py-2 bg-gray-200 rounded-md">Cancel</button> <button type="submit" class="px-4 py-2 bg-indigo-600 text-white rounded-md">Save</button> </div>
                </div>
            </form>
        </div>
    </div>
 <script>
        const ESP_HOSTNAME = window.location.hostname;
        const ROOMS_KEY = 'smartHomeRooms';
        const DEVICES_KEY_PREFIX = 'smartHomeDevices_';
        const ROOM_ICONS = ["fa-solid fa-lightbulb", "fa-solid fa-bed", "fa-solid fa-couch", "fa-solid fa-kitchen-set", "fa-solid fa-bath", "fa-solid fa-utensils", "fa-solid fa-desktop", "fa-solid fa-car", "fa-solid fa-gamepad", "fa-solid fa-shirt", "fa-solid fa-tree", "fa-solid fa-film", "fa-solid fa-dumbbell", "fa-solid fa-book", "fa-solid fa-children", "fa-solid fa-warehouse"];
        const DEVICE_ICONS = { light: 'lightbulb', fan: 'fan', ac: 'air-vent', tv: 'tv', lamp: 'lamp', spotlight: 'projector', outlet: 'plug', speaker: 'speaker', wifi: 'wifi', geyser: 'shower-head', power: 'power', desktop: 'monitor' };
        let rooms = [];
        let devices = [];
        let currentRoomId = null;
        let editingRoomId = null;
        let activeDevice = null;
        let originalGpio = null;
        const homeView = document.getElementById('home-view');
        const roomView = document.getElementById('room-view');
        const roomsGrid = document.getElementById('rooms-grid');
        const deviceGrid = document.getElementById('device-grid');
        const roomTitle = document.getElementById('room-title');
        const roomModifyModal = document.getElementById('room-modify-modal');
        const roomNameInput = document.getElementById('roomName');
        const roomIdDisplay = document.getElementById('room-id-display');
        const deviceModifyModal = document.getElementById('device-modify-modal');
        const deviceModifyForm = document.getElementById('modify-form');
        const scheduleModal = document.getElementById('schedule-modal');
        const scheduleForm = document.getElementById('schedule-form');
        
        async function apiGet(endpoint) { try { const response = await fetch(`http://${ESP_HOSTNAME}/${endpoint}`); if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`); return await response.json(); } catch (error) { console.error(`Could not fetch from ${endpoint}.`, error); return null; } }
        async function apiPost(endpoint, body) { try { await fetch(`http://${ESP_HOSTNAME}/${endpoint}`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }); } catch (error) { console.error(`Could not post to ${endpoint}.`, error); } }
        function saveToStorage(key, data) { localStorage.setItem(key, JSON.stringify(data)); }
        function loadFromStorage(key) { const data = localStorage.getItem(key); return data ? JSON.parse(data) : null; }
        function showView(viewId) { document.querySelectorAll('.view').forEach(v => v.classList.remove('active')); const view = document.getElementById(viewId); if (view) view.classList.add('active'); const navButtons = document.querySelectorAll('.nav-btn'); navButtons.forEach(b => b.classList.remove('active')); if (viewId === 'home-view') { navButtons[0].classList.add('active'); } }
        
        // --- THIS IS THE OLD FUNCTION, WE KEEP IT ---
        // --- BUT IT'S NO LONGER USED FOR INITIAL LOAD ---
        async function navigateToRoomView(roomId) { 
            currentRoomId = roomId; 
            const room = rooms.find(r => r.id === roomId); 
            roomTitle.textContent = room ? room.name : 'Room'; 
            const devicesKey = `${DEVICES_KEY_PREFIX}${roomId}`; 
            
            // This part is fine. It will now load the data
            // that syncAllDataFromESP() already saved.
            let storedDevices = loadFromStorage(devicesKey); 
            
            // This 'if' block is now just a fallback
            // in case the initial sync failed.
            if (!storedDevices) { 
                console.warn(`No devices found in localStorage for ${roomId}. Falling back to old API.`);
                const initialDevices = await apiGet(`devices?roomId=${roomId}`); 
                if (initialDevices) { 
                    storedDevices = initialDevices.map((dev, i) => ({ 
                        gpio: dev.gpio, 
                        name: `Switch ${i + 1}`, 
                        icon: 'light', 
                        hasSlider: dev.hasSlider, 
                        state: 'off', 
                        value: 0, 
                        schedule: { onTime: null, offTime: null } 
                    })); 
                    saveToStorage(devicesKey, storedDevices); 
                } 
            } 
            
            devices = storedDevices || []; 
            renderDevices(); 
            showView('room-view'); 
        }

        function renderRooms() { roomsGrid.innerHTML = ''; rooms.forEach(room => { const roomBtn = document.createElement('button'); roomBtn.className = 'room-btn'; roomBtn.onclick = () => navigateToRoomView(room.id); roomBtn.innerHTML = `<i class="${room.icon}"></i> ${room.name} <button class="room-options-btn"></button>`; const optionsBtn = roomBtn.querySelector('.room-options-btn'); const menu = document.createElement('div'); menu.className = 'room-options-menu'; menu.innerHTML = `<button class="edit-room-btn">Edit</button>`; menu.querySelector('.edit-room-btn').onclick = (e) => { e.stopPropagation(); openRoomEditModal(room.id); }; roomBtn.appendChild(menu); optionsBtn.onclick = (e) => { e.stopPropagation(); menu.classList.toggle('show'); }; roomsGrid.appendChild(roomBtn); }); }
        function openRoomEditModal(roomId) { editingRoomId = roomId; const room = rooms.find(r => r.id === roomId); if (!room) return; roomNameInput.value = room.name; roomIdDisplay.textContent = `ID: ${room.id}`; populateIconPicker('room-icon-list', ROOM_ICONS, room.icon, (iconClass) => room.icon = iconClass); roomModifyModal.style.display = 'flex'; }
        document.getElementById('save-room-btn').addEventListener('click', () => { const roomIndex = rooms.findIndex(r => r.id === editingRoomId); if (roomIndex > -1) { const room = rooms[roomIndex]; room.name = roomNameInput.value.trim(); const selectedIcon = document.querySelector('#room-icon-list .selected'); if(selectedIcon) room.icon = selectedIcon.dataset.icon; } saveToStorage(ROOMS_KEY, rooms); renderRooms(); closeAllModals(); });
        function renderDevices() { deviceGrid.innerHTML = ''; devices.forEach(device => { const card = document.createElement('div'); card.id = `device-${device.gpio}`; card.className = `device-card ${device.state} aspect-[1/1] rounded-2xl p-4 flex flex-col justify-between cursor-pointer`; card.onclick = (e) => { if (!e.target.closest('input, .options-button, .options-menu')) { toggleDeviceState(device); } }; renderDeviceContent(card, device); deviceGrid.appendChild(card); }); lucide.createIcons(); }
        function renderDeviceContent(card, device) { const iconName = DEVICE_ICONS[device.icon] || 'help-circle'; card.innerHTML = ` <div class="absolute top-2 right-2 z-10"> <button class="options-button p-2 rounded-full hover:bg-black/10"><i data-lucide="more-vertical" class="w-5 h-5"></i></button> <div class="options-menu hidden absolute right-0 mt-1 w-32 bg-white rounded-md shadow-lg z-20"> <a href="#" class="schedule-btn block px-4 py-2 text-sm text-gray-700 hover:bg-gray-100">Schedule</a> <a href="#" class="modify-btn block px-4 py-2 text-sm text-gray-700 hover:bg-gray-100">Modify</a> </div> </div> <div class="flex h-full w-full"> <div class="flex flex-col justify-between flex-grow"> <div><i data-lucide="${iconName}" class="w-8 h-8"></i><h3 class="font-bold text-lg mt-2">${device.name}</h3></div> <div><div class="schedule-info opacity-70"></div></div> </div> ${device.hasSlider ? `<div class="flex justify-center pl-2 pt-10"><input type="range" min="0" max="255" value="${device.value}" class="vertical-slider"></div>` : ''} </div>`; card.querySelector('.options-button').onclick = e => { e.stopPropagation(); card.querySelector('.options-menu').classList.toggle('hidden'); }; card.querySelector('.schedule-btn').onclick = e => { e.stopPropagation(); openScheduleModal(device); }; card.querySelector('.modify-btn').onclick = e => { e.stopPropagation(); openDeviceModifyModal(device); }; 
        if (device.hasSlider) {
            const slider = card.querySelector('.vertical-slider');
            slider.oninput = (e) => { 
                e.stopPropagation(); 
                device.value = parseInt(e.target.value); 
            }; 
            
            slider.onchange = (e) => { 
                e.stopPropagation(); 
                const newValue = parseInt(e.target.value);
                const newState = (newValue > 0) ? 'on' : 'off';
                updateDeviceUI(device, newState, newValue); 
                sendDeviceCommand(device); 
            }; 
        }
         updateScheduleInfo(card, device); }
        function updateScheduleInfo(card, device) { const on = device.schedule.onTime; const off = device.schedule.offTime; let text = []; if (on) text.push(`ON: ${on}`); if (off) text.push(`OFF: ${off}`); card.querySelector('.schedule-info').innerHTML = text.join('<br>'); }
        
       function updateDeviceUI(device, newState, newValue, newHasSlider) {
            let needsFullRender = false;

            // 1. Update the in-memory device object
            if (newState !== undefined) {
                device.state = newState;
            }
            if (newValue !== undefined) {
                device.value = newValue;
            }

            // --- NEW ---
            // Check if the slider's existence has changed
            if (newHasSlider !== undefined && newHasSlider !== null && device.hasSlider !== newHasSlider) {
                console.log(`GPIO ${device.gpio}: Slider state changed to ${newHasSlider}`);
                device.hasSlider = newHasSlider;
                needsFullRender = true; // Mark for a full re-render
            }
            // --- END NEW ---

            // 2. Find the card in the DOM
            const card = document.getElementById(`device-${device.gpio}`);
            if (card) {
                // --- NEW LOGIC ---
                if (needsFullRender) {
                    // Re-render the entire card's content to add/remove the slider
                    renderDeviceContent(card, device);
                    lucide.createIcons(); // Re-init icons
                }
                // --- END NEW LOGIC ---

                // This part always runs to update the on/off state
                card.className = `device-card ${device.state} aspect-[1/1] rounded-2xl p-4 flex flex-col justify-between cursor-pointer`;
                
                // Update slider value (if it exists and we didn't just re-render)
                if (device.hasSlider && !needsFullRender) {
                    const slider = card.querySelector('.vertical-slider');
                    if (slider) {
                        slider.value = device.value;
                    }
                }
            }
            
            // 5. Save all changes to localStorage
            const deviceIndex = devices.findIndex(d => d.gpio === device.gpio);
            if(deviceIndex > -1) {
                // This updates the 'devices' array for the save
                devices[deviceIndex] = device;
            }
            saveToStorage(`${DEVICES_KEY_PREFIX}${currentRoomId}`, devices);
        }

        function sendDeviceCommand(device) {
            let stateToSend = 0;
            let valueToSend =device.value;
            let sliderToSend = device.hasSlider;
          
            if (device.state === 'on') {
              stateToSend = 1;
            }
            
            apiPost('control', { 
                roomId: currentRoomId, 
                gpio: device.gpio, 
                state: stateToSend, 
                slider: sliderToSend,
                value: valueToSend 
            });
        }
        
        function toggleDeviceState(device) {
            const newState = (device.state === 'on' ? 'off' : 'on');
            let newValue = device.value;

            if (newState === 'on' && device.value === 0 && device.hasSlider) {
                newValue = 255;
            }
            updateDeviceUI(device, newState, newValue);
            sendDeviceCommand(device);
        }

        function openDeviceModifyModal(device) { activeDevice = device; originalGpio = device.gpio; document.getElementById('deviceName').value = device.name; document.getElementById('devicePin').value = device.gpio; document.getElementById('hasSlider').checked = device.hasSlider; populateIconPicker('device-icon-selection', Object.keys(DEVICE_ICONS), device.icon, (iconKey) => {}); deviceModifyModal.style.display = 'flex'; }
        deviceModifyForm.addEventListener('submit', (e) => { e.preventDefault(); const deviceIndex = devices.findIndex(d => d.gpio === originalGpio); if (deviceIndex === -1) return; devices[deviceIndex].name = document.getElementById('deviceName').value.trim(); devices[deviceIndex].gpio = parseInt(document.getElementById('devicePin').value); devices[deviceIndex].hasSlider = document.getElementById('hasSlider').checked; const selectedIcon = document.querySelector('#device-icon-selection .selected'); if(selectedIcon) devices[deviceIndex].icon = selectedIcon.dataset.icon; saveToStorage(`${DEVICES_KEY_PREFIX}${currentRoomId}`, devices); renderDevices(); closeAllModals(); });
       
        function convertTimeToMinutes(timeString) {
          if (!timeString) {
            return 9999;
          }
          const parts = timeString.split(':');
          const hours = parseInt(parts[0], 10);
          const minutes = parseInt(parts[1], 10);
          
          if (isNaN(hours) || isNaN(minutes)) {
            return 9999;
          }
          
          return (hours * 60) + minutes;
        }
       
        function openScheduleModal(device) { activeDevice = device; document.getElementById('schedule-modal-title').textContent = `Schedule for ${device.name}`; const { onTime, offTime } = device.schedule; document.getElementById('scheduleOnEnabled').checked = !!onTime; document.getElementById('scheduleOffEnabled').checked = !!offTime; document.getElementById('scheduleOnTime').value = onTime || ''; document.getElementById('scheduleOffTime').value = offTime || ''; scheduleModal.style.display = 'flex'; }
        scheduleForm.addEventListener('submit', (e) => { e.preventDefault(); if (!activeDevice) return; const deviceIndex = devices.findIndex(d => d.gpio === activeDevice.gpio); if (deviceIndex === -1) return; const onTime = document.getElementById('scheduleOnEnabled').checked ? document.getElementById('scheduleOnTime').value : null; const offTime = document.getElementById('scheduleOffEnabled').checked ? document.getElementById('scheduleOffTime').value : null;const onTimeInMinutes = convertTimeToMinutes(onTime); const offTimeInMinutes = convertTimeToMinutes(offTime);  devices[deviceIndex].schedule.onTime = onTime; devices[deviceIndex].schedule.offTime = offTime; apiPost('schedule', { roomId: currentRoomId, gpio: activeDevice.gpio, onTime: onTimeInMinutes, offTime: offTimeInMinutes }); saveToStorage(`${DEVICES_KEY_PREFIX}${currentRoomId}`, devices); renderDevices(); closeAllModals(); });
        document.getElementById('clear-schedule').addEventListener('click', () => { if (!activeDevice) return; const deviceIndex = devices.findIndex(d => d.gpio === activeDevice.gpio); if (deviceIndex === -1) return; devices[deviceIndex].schedule = { onTime: null, offTime: null }; apiPost('schedule', { roomId: currentRoomId, gpio: activeDevice.gpio, onTime: 9999, offTime: 9999 }); saveToStorage(`${DEVICES_KEY_PREFIX}${currentRoomId}`, devices); renderDevices(); closeAllModals(); });
        function checkSchedules() { if (!devices.length || !currentRoomId) return; const now = new Date(); const currentTime = `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}`; devices.forEach(device => { if (device.schedule.onTime === currentTime && device.state === 'off') toggleDeviceState(device); if (device.schedule.offTime === currentTime && device.state === 'on') toggleDeviceState(device); }); }
        function populateIconPicker(containerId, iconList, currentIcon, callback) { const container = document.getElementById(containerId); container.innerHTML = ''; iconList.forEach(icon => { const wrapper = document.createElement('div'); wrapper.className = 'icon-wrapper p-2'; wrapper.dataset.icon = icon; if(containerId === 'room-icon-list') { wrapper.innerHTML = `<i class="${icon} text-2xl"></i>`; } else { wrapper.innerHTML = `<i data-lucide="${DEVICE_ICONS[icon]}" class="w-6 h-6"></i>`; } if (icon === currentIcon) wrapper.classList.add('selected'); wrapper.onclick = () => { container.querySelector('.selected')?.classList.remove('selected'); wrapper.classList.add('selected'); callback(icon); }; container.appendChild(wrapper); }); if(containerId.includes('device')) lucide.createIcons(); }
        function closeAllModals() { document.querySelectorAll('.modal-overlay').forEach(m => m.style.display = 'none'); editingRoomId = null; activeDevice = null; originalGpio = null; }
        async function updateStatusDisplay() { const statusData = await apiGet('status'); if (statusData) { document.getElementById('voltage-display').textContent = `${statusData.voltage.toFixed(0)}V`; document.getElementById('pf-display').textContent = statusData.pf.toFixed(2); document.getElementById('wattage-display').textContent = `${statusData.wattage.toFixed(0)}W`; } }
        document.querySelectorAll('.cancel-btn').forEach(btn => btn.onclick = closeAllModals);
        document.addEventListener('click', (event) => { if (!event.target.closest('.room-options-btn, .options-button')) { document.querySelectorAll('.room-options-menu.show, .options-menu:not(.hidden)').forEach(m => m.classList.remove('show') || m.classList.add('hidden')); } });

        // ===================================================================
        // --- THIS IS THE NEW CODE YOU WERE MISSING ---
        // ===================================================================

        /**
         * NEW FUNCTION: Fetches the raw config and states from the ESP
         * and builds the entire localStorage database.
         */
       /**
         * NEW FUNCTION (REPLACEMENT): Fetches raw config and states, 
         * then MERGES them with existing data in localStorage.
         */
        async function syncAllDataFromESP() {
        console.log("Fetching config and states from ESP in parallel...");
        
        const [configData, rawStates] = await Promise.all([
            apiGet('get-config-maps'),
            apiGet('get-raw-states')
        ]);
        
        if (!configData || !rawStates || !configData.rooms) {
            console.error("Failed to load initial data from ESP.");
            return null; // Return null to show we failed
        }

        let stateIndex = 0; // This is our new counter

        // --- THIS IS THE START OF THE MERGE LOGIC ---

        // 1. Load old room config (to preserve names/icons)
        const oldRoomConfig = loadFromStorage(ROOMS_KEY) || [];
        const newRoomConfig = []; 

        // 2. Loop through the new room config from ESP
        for (const roomData of configData.rooms) {
            const roomId = roomData.id;
            const pinsForThisRoom = roomData.pins;
            
            // --- THIS WAS MISSING ---
            // Find the old room data
            const oldRoom = oldRoomConfig.find(r => r.id === roomId);
            
            // Create the merged room object
            const mergedRoom = {
                id: roomId,
                name: oldRoom ? oldRoom.name : `Room ${roomId}`, // Use old name if it exists
                icon: oldRoom ? oldRoom.icon : ROOM_ICONS[newRoomConfig.length % ROOM_ICONS.length] // Use old icon
            };
            newRoomConfig.push(mergedRoom); // Add to our list
            // --- END OF MISSING ROOM LOGIC ---
            
            const devicesKey = `${DEVICES_KEY_PREFIX}${roomId}`;
            const oldDeviceList = loadFromStorage(devicesKey) || [];
            let newDeviceList = [];

            // 3. Loop through the pins *for this room*
            for (let b = 0; b < pinsForThisRoom.length; b++) {
                const gpio = pinsForThisRoom[b];
                const newState = rawStates[stateIndex]; 
                
                if (!newState) {
                    console.error(`State mismatch for ${roomId} GPIO ${gpio}`);
                    continue; // Avoid a crash if C++ and JS are out of sync
                }

                // --- THIS WAS THE LINE CAUSING THE CRASH ---
                // Find the old device data
                const oldDevice = oldDeviceList.find(d => d.gpio === gpio);
                // --- END OF MISSING DEVICE LOGIC ---

                // NOW, 'oldDevice' is defined (it's either the device or 'undefined', both are fine)
                newDeviceList.push({
                    // Keep old data if it exists
                    gpio: gpio,
                    name: oldDevice ? oldDevice.name : `Switch ${b + 1}`, // <-- This will no longer crash
                    icon: oldDevice ? oldDevice.icon : 'light',
                    schedule: oldDevice ? oldDevice.schedule : { onTime: null, offTime: null },

                    // Overwrite with fresh data from ESP
                    hasSlider: newState.s,
                    state: (newState.p > 0) ? 'on' : 'off',
                    value: newState.v
                });

                // 4. CRITICAL: Increment the counter for the *next* state
                stateIndex++; 
            }
            // Save this room's merged device list
            saveToStorage(devicesKey, newDeviceList);
        }
        
        // --- THIS WAS ALSO MISSING ---
        // Save the newly merged *room* list (with old names/icons)
        saveToStorage(ROOMS_KEY, newRoomConfig);
        // --- END OF MISSING SAVE ---

        console.log("All device data MERGED from ESP to localStorage.");
        return newRoomConfig; // Return the merged room config
    }

        /**
         * THIS IS THE NEW, REPLACED initialize() FUNCTION
         */
       /**
         * THIS IS THE NEW, REPLACED initialize() FUNCTION
         * It ALWAYS fetches from the ESP on page load.
         */
        /**
         * THIS IS THE CORRECT initialize() FUNCTION
         * It ALWAYS fetches from the ESP on page load.
         */
        async function initialize() {
            document.getElementById('back-to-home').addEventListener('click', () => { currentRoomId = null; showView('home-view'); });

            // 1. ALWAYS run the full sync from the ESP on page load
            console.log("Running full sync from ESP...");
            let syncedRooms = await syncAllDataFromESP(); // This will now merge data
            
            if (!syncedRooms) {
                // Handle error: ESP might be offline
                console.error("Could not sync from ESP. Trying to load from cache...");
                
                // Try to load the old data from storage as a fallback
                rooms = loadFromStorage(ROOMS_KEY) || [];
            } else {
                rooms = syncedRooms;
            }

            // 2. Load rooms and render
            renderRooms();
            
            // 3. Setup intervals (existing code)
            // setInterval(checkSchedules, 30000);
            updateStatusDisplay();
            setInterval(updateStatusDisplay, 5000);
            showView('home-view');
        
            // 4. Setup SSE Listener (This logic is 100% unchanged)
            const evtSource = new EventSource(`http://${ESP_HOSTNAME}/events`);

          evtSource.addEventListener('stateChange', (event) => {
            console.log("SSE Received:", event.data);
            try {
                const data = JSON.parse(event.data);
                const newStateString = data.state ? 'on' : 'off';
                
                // --- CORRECTED ---
                // Check for the 'slider' property from your SSE
                const newHasSlider = (data.slider !== undefined) ? data.slider : null; 
                // --- END CORRECTION ---

                if (data.roomId === currentRoomId) {
                    // Device is in the currently active room
                    const device = devices.find(d => d.gpio === data.gpio);
                    if (device) {
                        // Pass all new data to the UI function
                        // (My previous updateDeviceUI function handles this)
                        updateDeviceUI(device, newStateString, data.value, newHasSlider);
                    }
                } else {
                    // Device is in a different room (background update)
                    const devicesKey = `${DEVICES_KEY_PREFIX}${data.roomId}`;
                    let storedDevices = loadFromStorage(devicesKey);
                    if (storedDevices) {
                        const deviceIndex = storedDevices.findIndex(d => d.gpio === data.gpio);
                        if (deviceIndex > -1) {
                            // Update the state from 0/1 to 'on'/'off'
                            storedDevices[deviceIndex].state = newStateString; 
                            storedDevices[deviceIndex].value = data.value;
                            
                            // --- CORRECTED ---
                            if (newHasSlider !== null) {
                                storedDevices[deviceIndex].hasSlider = newHasSlider;
                            }
                            // --- END CORRECTION ---
                            
                            saveToStorage(devicesKey, storedDevices);
                        }
                    }
                }
            } catch (error) {
                console.error('Error parsing SSE data:', error);
            }
        });

            evtSource.onerror = (err) => {
                console.error("EventSource failed:", err);
            };
        }
        
        // --- END OF NEW CODE ---

        document.addEventListener('DOMContentLoaded', initialize);
    </script>
</body>
</html>
)RAW_HTML";

const char POWER_HTML_PROGMEM[] PROGMEM = R"RAW_HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Power Consumption Dashboard</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link rel="icon" href="https://cdn-icons-png.flaticon.com/512/1427/1427247.png" type="image/x-icon">
    <link rel="apple-touch-icon" href="https://cdn-icons-png.flaticon.com/512/1427/1427247.png">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        body { font-family: 'Inter', sans-serif; background-color: #f3f4f6; color: #1f2937; margin: 0; display: flex; justify-content: center; min-height: 100vh; line-height: 1.5; padding-bottom: 80px; }
        .container { width: 95%; max-width: 1100px; padding: 2vw; box-sizing: border-box; }
        header { text-align: center; margin-bottom: 3rem; }
        header h1 { font-size: clamp(2rem, 5vw, 3rem); font-weight: 700; color: #0891b2; margin: 0; }
        header p { color: #4b5563; font-size: clamp(1rem, 2vw, 1.25rem); margin-top: 0.5rem; }
        .section-title { font-size: clamp(1.25rem, 3vw, 1.75rem); font-weight: 600; color: #374151; margin-bottom: 1.5rem; border-bottom: 2px solid #e5e7eb; padding-bottom: 0.5rem; }
        .overall-consumption-layout { display: grid; grid-template-columns: repeat(2, 1fr); gap: 1.5vw; margin-bottom: 50px; }
        .stat-card { background-color: #ffffff; padding:10px; border-radius: clamp(1rem, 1.5vw, 1.25rem); box-shadow: 0 4px 20px -5px rgba(0, 0, 0, 0.1); display: flex; align-items: center; justify-content: space-between; }
        .stat-card .stat-label { font-size: clamp(0.8rem, 1.5vw, 1rem); color: #6b7280; margin: 0; }
        .stat-card .stat-value { font-weight: 600; color: #111827; font-size: clamp(1.75rem, 4vw, 2.5rem); margin: 0; }
        .stat-card .stat-icon { font-size: clamp(2rem, 4vw, 2.75rem); }
        .icon-power { color: #22c55e; } .icon-voltage { color: #facc15; } .icon-ampere { color: #ef4444; } .icon-pf { color: #8b5cf6; }
        .room-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); gap: 1.5vw; }
        .room-card { background-color: #ffffff; border-radius: 50%; border: 1px solid #e5e7eb; box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -2px rgba(0, 0, 0, 0.1); display: flex; flex-direction: column; align-items: center; justify-content: center; aspect-ratio: 1 / 1; text-align: center; padding: 1rem; transition: transform 0.2s ease, box-shadow 0.2s ease; }
        .room-card:hover { transform: translateY(-5px); box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -4px rgba(0, 0, 0, 0.1); }
        .room-card h3 { font-weight: 600; font-size: clamp(0.9rem, 2.5vw, 1.1rem); margin: 0; }
        .room-card .power-value { font-weight: 700; font-size: clamp(1.5rem, 4vw, 2rem); color: #0891b2; margin: 0.25rem 0; }
        .room-card .ampere-value { font-size: clamp(0.8rem, 2vw, 1rem); color: #6b7280; margin: 0; }
        .bottom-nav { position: fixed; bottom: 0; left: 0; width: 100%; background: white; display: flex; justify-content: space-around; padding: 12px 0; border-top: 1px solid #ddd; z-index: 100; }
        .nav-btn { background: none; border: none; cursor: pointer; font-size: 20px; color: #666; transition: 0.3s; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Live Power Dashboard</h1>
            <p>Real-time energy monitoring for your home</p>
        </header>
        <section>
            <h2 class="section-title">Overall Consumption</h2>
            <div class="overall-consumption-layout">
                <div class="stat-card">
                    <div>
                        <p class="stat-label">Total Power</p>
                        <p id="total-power" class="stat-value">0 W</p>
                    </div>
                    <i class="fa-solid fa-plug-circle-bolt stat-icon icon-power"></i>
                </div>
                <div class="stat-card">
                    <div>
                        <p class="stat-label">Voltage</p>
                        <p id="total-voltage" class="stat-value">0 V</p>
                    </div>
                    <i class="fa-solid fa-bolt-lightning stat-icon icon-voltage"></i>
                </div>
                <div class="stat-card">
                    <div>
                        <p class="stat-label">Total Amperage</p>
                        <p id="total-ampere" class="stat-value">0.00 A</p>
                    </div>
                    <i class="fa-solid fa-wave-square stat-icon icon-ampere"></i>
                </div>
                <div class="stat-card">
                    <div>
                        <p class="stat-label">Power Factor</p>
                        <p id="power-factor" class="stat-value">0.00</p>
                    </div>
                    <i class="fa-solid fa-chart-line stat-icon icon-pf"></i>
                </div>
            </div>
        </section>
        <section>
            <h2 class="section-title">Usage by Room</h2>
            <div id="room-grid" class="room-grid"></div>
        </section>
        <div class="bottom-nav">
            <button class="nav-btn" onclick="window.location.href='/'"> <i class="fa-solid fa-house"></i></button>
            <button class="nav-btn" style="color: #3e67bf"> <i class="fa-solid fa-bolt"></i></button>
            <button class="nav-btn" onclick="window.location.href='/graph'"><i class="fa-solid fa-chart-column"></i></button>
            <button class="nav-btn" onclick="window.location.href='/settings'"> <i class="fa-solid fa-gear"></i></button>
        </div>
    </div>
    <script>
        document.addEventListener('DOMContentLoaded', () => {
            const ESP_HOSTNAME = window.location.hostname;
            const ROOMS_KEY = 'smartHomeRooms';
            const roomGrid = document.getElementById('room-grid');
            function initializeRoomUI() {
                const storedRooms = JSON.parse(localStorage.getItem(ROOMS_KEY)) || [];
                if (storedRooms.length === 0) { roomGrid.innerHTML = '<p>No room data found. Please set up rooms on the home panel first.</p>'; return; }
                roomGrid.innerHTML = '';
                storedRooms.forEach(room => {
                    const roomEl = document.createElement('div');
                    roomEl.className = 'room-card';
                    roomEl.innerHTML = `<h3>${room.name}</h3> <p id="power-${room.id}" class="power-value">0W</p> <p id="ampere-${room.id}" class="ampere-value">0.00A</p>`;
                    roomGrid.appendChild(roomEl);
                });
            }
            async function fetchAndDisplayPowerData() {
                try {
                    const [statusRes, roomPowerRes] = await Promise.all([ fetch(`http://${ESP_HOSTNAME}/status`), fetch(`http://${ESP_HOSTNAME}/roompower`) ]);
                    if (!statusRes.ok || !roomPowerRes.ok) { throw new Error('Failed to fetch data from ESP'); }
                    const statusData = await statusRes.json();
                    const roomPowerData = await roomPowerRes.json();
                    let totalAmps = statusData.wattage / statusData.voltage;
                    document.getElementById('total-power').textContent = `${statusData.wattage.toFixed(0)} W`;
                    document.getElementById('total-voltage').textContent = `${statusData.voltage.toFixed(0)} V`;
                    document.getElementById('total-ampere').textContent = `${totalAmps.toFixed(2)} A`;
                    document.getElementById('power-factor').textContent = statusData.pf.toFixed(2);
                    roomPowerData.forEach(room => {
                        const powerEl = document.getElementById(`power-${room.id}`);
                        const ampereEl = document.getElementById(`ampere-${room.id}`);
                        if (powerEl && ampereEl) { powerEl.textContent = `${room.power.toFixed(0)}W`; ampereEl.textContent = `${room.ampere.toFixed(2)}A`; }
                    });
                } catch (error) { console.error("Error updating power data:", error); }
            }
            initializeRoomUI();
            fetchAndDisplayPowerData();
            setInterval(fetchAndDisplayPowerData, 3000);
        });
    </script>
</body>
</html>
)RAW_HTML";

const char SETTINGS_HTML_PROGMEM[] PROGMEM = R"RAW_HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Home Safety Dashboard</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
     <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css">
    <style>
        body { font-family: 'Inter', sans-serif; background-color: #f3f4f6; color: #111827; padding-bottom: 80px; }
        .toggle-checkbox:checked { right: 0; border-color: #22c55e; }
        .toggle-checkbox:checked + .toggle-label { background-color: #22c55e; }
        .disabled-card { background-color: #e5e7eb; opacity: 0.7; }
        .disabled-card input[type="number"] { cursor: not-allowed; background-color: #d1d5db; }
        input[type=number]::-webkit-inner-spin-button, input[type=number]::-webkit-outer-spin-button { -webkit-appearance: none; margin: 0; }
        input[type=number] { -moz-appearance: textfield; }
        .bottom-nav { position: fixed; bottom: 0; left: 0; width: 100%; background: white; display: flex; justify-content: space-around; padding: 12px 0; border-top: 1px solid #ddd; z-index: 100; }
        .nav-btn { background: none; border: none; cursor: pointer; font-size: 20px; color: #666; transition: 0.3s; }
    </style>
</head>
<body class="antialiased">
    <div class="min-h-screen p-4 sm:p-6 lg:p-8">
        <div class="max-w-7xl mx-auto">
            <header class="text-center mb-8 md:mb-12">
                <h1 class="text-3xl sm:text-4xl lg:text-5xl font-bold text-gray-900">Smart Home Safety</h1>
                <p class="mt-2 text-lg text-gray-600">Manage auto-cutoff settings for voltage and current.</p>
            </header>
            <main>
                <div class="bg-white rounded-2xl p-6 shadow-lg mb-8 border border-gray-200">
                    <h2 class="text-2xl font-semibold text-gray-900 mb-4">Global House Settings</h2>
                    <p class="text-gray-500 mb-6">These are the master settings for your entire home.</p>
                    <div class="grid grid-cols-1 gap-4">
                        <div class="mx-auto">
                            <label for="globalVoltage" class="text-base font-medium text-gray-700">Max Voltage (V)</label>
                            <div class="mt-2 flex items-center border border-gray-300 rounded-lg overflow-hidden focus-within:ring-2 focus-within:ring-blue-500">
                                <input type="number" id="globalVoltage" value="240" class="w-full p-3 bg-gray-50 outline-none border-none">
                            </div>
                        </div>
                        <div class="mx-auto">
                            <label for="globalCurrent" class="text-base font-medium text-gray-700">Max Current (A)</label>
                            <div class="mt-2 flex items-center border border-gray-300 rounded-lg overflow-hidden focus-within:ring-2 focus-within:ring-blue-500">
                               <input type="number" id="globalCurrent" value="15" class="w-full p-3 bg-gray-50 outline-none border-none">
                            </div>
                        </div>
                    </div>
                    <div class="mt-6 text-right">
                         <button id="save-global-btn" class="bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-5 rounded-lg transition-colors duration-300">Save Global Settings</button>
                    </div>
                </div>
                <div>
                    <h2 class="text-2xl font-semibold text-gray-900 mb-6">Room-Specific Overrides</h2>
                    <div id="room-grid" class="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-6">
                        </div>
                </div>
            </main>
        </div>
    </div>
    <div id="toast" class="fixed bottom-20 right-5 bg-green-600 text-white py-2 px-4 rounded-lg shadow-xl translate-x-[120%] transition-transform duration-300 ease-in-out">
        Settings saved successfully!
    </div>
    <div class="bottom-nav">
        <button class="nav-btn" onclick="window.location.href='/'"> <i class="fa-solid fa-house"></i></button>
        <button class="nav-btn" onclick="window.location.href='/power'"> <i class="fa-solid fa-bolt"></i></button>
        <button class="nav-btn" onclick="window.location.href='/graph'"><i class="fa-solid fa-chart-column"></i></button>
        <button class="nav-btn" style="color: #3e67bf"> <i class="fa-solid fa-gear"></i></button>
    </div>
    <script>
    document.addEventListener('DOMContentLoaded', async () => {
        const ESP_HOSTNAME = window.location.hostname;
        const ROOMS_KEY = 'smartHomeRooms';
        const GLOBAL_ROOM_ID = 222; // This is used for saving global settings

        // --- Get HTML Elements ---
        const globalVoltageInput = document.getElementById('globalVoltage');
        const globalCurrentInput = document.getElementById('globalCurrent');
        const roomGrid = document.getElementById('room-grid');

        // --- API Functions ---
        async function apiGet(endpoint) { 
            try { 
                const response = await fetch(`http://${ESP_HOSTNAME}/${endpoint}`); 
                if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`); 
                return await response.json(); 
            } catch (error) { 
                console.error(`Could not fetch from ${endpoint}.`, error); 
                return null; 
            } 
        }
        
        async function apiPost(endpoint, body) { 
            try { 
                await fetch(`http://${ESP_HOSTNAME}/${endpoint}`, { 
                    method: 'POST', 
                    headers: { 'Content-Type': 'application/json' }, 
                    body: JSON.stringify(body) 
                }); 
                showToast(); 
            } catch (error) { 
                console.error(`Could not post to ${endpoint}.`, error); 
            } 
        }

        // --- UI Functions ---
        function showToast() {
            const toast = document.getElementById('toast');
            toast.classList.remove('translate-x-[120%]');
            setTimeout(() => { toast.classList.add('translate-x-[120%]'); }, 2000);
        }

        function toggleRoomState(card, isEnabled) {
            const voltageInput = card.querySelector('.room-voltage-input');
            const currentInput = card.querySelector('.room-current-input');
            card.classList.toggle('disabled-card', !isEnabled);
            voltageInput.disabled = !isEnabled;
            currentInput.disabled = !isEnabled;
            if (!isEnabled) {
                voltageInput.value = globalVoltageInput.value;
                currentInput.value = globalCurrentInput.value;
            }
        }

        /**
         * This function renders the safety settings UI.
         */
        function renderUI(settings, storedRooms) {
            // 1. Populate Global Settings
            globalVoltageInput.value = settings.global.voltage;
            globalCurrentInput.value = settings.global.current;

            // 2. Populate Room-Specific Cards
            roomGrid.innerHTML = '';
            settings.rooms.forEach(roomSetting => {
                // Find the room name from localStorage (saved by your other app)
                const roomInfo = storedRooms.find(r => r.id == roomSetting.id) || { name: `Room ${roomSetting.id}` };
                
                const card = document.createElement('div');
                card.id = `room-${roomSetting.id}`;
                card.className = 'room-card bg-white rounded-2xl p-6 shadow-lg border border-gray-200 transition-opacity duration-300';
                card.dataset.roomId = roomSetting.id;

                card.innerHTML = `
                    <div class="flex justify-between items-start">
                        <h3 class="text-xl font-bold text-gray-900">${roomInfo.name}</h3>
                        <div class="relative inline-block w-10 ml-2 align-middle select-none transition duration-200 ease-in">
                            <input type="checkbox" class="toggle-checkbox absolute block w-6 h-6 rounded-full bg-white border-4 appearance-none cursor-pointer" ${roomSetting.override ? 'checked' : ''}/>
                            <label class="toggle-label block overflow-hidden h-6 rounded-full bg-gray-300 cursor-pointer"></label>
                        </div>
                    </div>
                    <p class="text-sm text-gray-500 mb-6">Override global settings</p>
                    <div class="grid grid-cols-1 gap-4">
                        <div class="mx-auto">
                            <label class="text-sm font-medium text-gray-700">Max Voltage (V)</label>
                            <div class="mt-2 flex items-center border border-gray-300 rounded-lg overflow-hidden focus-within:ring-2 focus-within:ring-blue-500">
                                <input type="number" class="room-voltage-input w-full p-3 bg-gray-50 outline-none border-none" value="${roomSetting.voltage}">
                            </div>
                        </div>
                        <div class="mx-auto">
                            <label class="text-sm font-medium text-gray-700">Max Current (A)</label>
                            <div class="mt-2 flex items-center border border-gray-300 rounded-lg overflow-hidden focus-within:ring-2 focus-within:ring-blue-500">
                                <input type="number" class="room-current-input w-full p-3 bg-gray-50 outline-none border-none" value="${roomSetting.current}">
                            </div>
                        </div>
                    </div>
                    <div class="mt-6 text-right">
                        <button class="save-room-btn bg-blue-600 hover:bg-blue-700 text-white font-bold py-2 px-5 rounded-lg transition-colors duration-300">Save</button>
                    </div>`;
                
                roomGrid.appendChild(card);
                // Set the initial disabled state
                toggleRoomState(card, roomSetting.override);
            });
        }
        
        // --- Event Listeners ---
        document.getElementById('save-global-btn').addEventListener('click', () => {
             const payload = {
                 roomId: GLOBAL_ROOM_ID, // Special ID for global settings
                 voltage: parseInt(globalVoltageInput.value) || 0,
                 current: parseInt(globalCurrentInput.value) || 0
             };
             apiPost('setsetting', payload);
        });

        roomGrid.addEventListener('change', e => {
            if (e.target.matches('.toggle-checkbox')) {
                toggleRoomState(e.target.closest('.room-card'), e.target.checked);
            }
        });
        
        roomGrid.addEventListener('click', e => {
            if (e.target.matches('.save-room-btn')) {
                const card = e.target.closest('.room-card');
                const payload = {
                    roomId: parseInt(card.dataset.roomId),
                    voltage: parseInt(card.querySelector('.room-voltage-input').value) || 0,
                    current: parseInt(card.querySelector('.room-current-input').value) || 0,
                    override: card.querySelector('.toggle-checkbox').checked
                };
                apiPost('setsetting', payload);
            }
        });

        /**
         * THIS IS THE NEW, CORRECT INITIALIZE FUNCTION
         */
        async function initialize() {
            // 1. Load room names from localStorage (which your other app saves)
            const storedRooms = JSON.parse(localStorage.getItem(ROOMS_KEY)) || [];
            
            // 2. Fetch all safety settings from the ESP
            //    (You must create this '/get-all-settings' endpoint on your ESP)
            const allSettings = await apiGet('getsettings');

            if (allSettings && allSettings.global && allSettings.rooms) {
                // 3. If data is loaded, call renderUI to show everything
                renderUI(allSettings, storedRooms);
            } else {
                console.error("Could not load safety settings from ESP.");
                // Optionally show an error to the user
            }
        }
        
        // --- Run Initialization ---
        initialize();
        
    });
    </script>
</body>
</html>
)RAW_HTML";

// --- FINAL, CORRECTED: Graph page with dynamic data, name matching, and visible labels ---
const char GRAPH_HTML_PROGMEM[] PROGMEM = R"RAW_HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Power Consumption Dashboard</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css">
    <style>
        body { font-family: 'Inter', sans-serif; background-color: #f0f2f5; padding-bottom: 80px; }
        .chart-wrapper { position: relative; width:100vw; height: 280px; margin: 0 auto; }
        .chart-container { position: relative; width: 100%; height: 100%; }
        .chart-text-link { cursor: pointer; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); text-align: center; display: block; padding: 1rem; border-radius: 9999px; transition: background-color: 0.2s; }
        .chart-text-link:hover { background-color: #F3F4F6; }
        .chart-label { position: absolute; transform: translate(-50%, -50%); font-size: 0.75rem; color: #4B5563; }
        .room-grid { display: grid; grid-template-columns: 1fr 1fr ; gap: 0.75rem; margin-top: 1.5rem; max-width: 24rem; margin-left: auto; margin-right: auto; }
        .room-card { cursor: pointer; display: flex; flex-direction: column; align-items: center; justify-content: space-between; background-color: white; border-radius: 0.75rem; padding: 1rem; box-shadow: 0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1); transition: transform 0.2s, box-shadow 0.2s; text-decoration: none; color: inherit; border: 1px solid #e5e7eb; }
        .room-card:hover { transform: translateY(-3px); box-shadow: 0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1); }
        .time-range-btn { padding: 0.5rem 1rem; border-radius: 0.5rem; border: 1px solid #D1D5DB; background-color: white; color: #374151; font-weight: 500; transition: all 0.2s; }
        .time-range-btn.active { background-color: #4F46E5; color: white; border-color: #4F46E5; }
        .bottom-nav { position: fixed; bottom: 0; left: 0; width: 100%; background: white; display: flex; justify-content: space-around; padding: 12px 0; border-top: 1px solid #ddd; z-index: 100; }
        .nav-btn { background: none; border: none; cursor: pointer; font-size: 20px; color: #666; transition: 0.3s; }
    </style>
</head>
<body class="bg-white">

    <div id="main-dashboard" class="w-full text-center">
        <h1 class="text-2xl font-bold text-gray-800 mb-1">Today's Power</h1>
        <p class="text-sm text-gray-500 mb-6">Live data from your smart meter.</p>
        <div class="flex justify-center mb-6">
            <div id="chart-wrapper" class="chart-wrapper">
                <div class="chart-container">
                    <canvas id="consumptionChart"></canvas>
                    <a class="chart-text-link group" data-room-id="Total" data-room-name="Total">
                        <p class=" justify-start text-gray-500 text-xs">TOTAL</p>
                        <p id="totalConsumption" class="text-3xl font-bold text-gray-800 flex items-center justify-center"></p>
                    </a>
                </div>
            </div>
        </div>
        <div id="room-details" class="room-grid p-6"></div>
    </div>

    <div id="detail-view" class="pt-6 pb-6 w-full hidden">
        <header class="flex items-center mb-6 px-4">
            <button id="back-btn" class="p-2 rounded-full hover:bg-gray-100 mr-2">
                <svg xmlns="http://www.w3.org/2000/svg" class="h-6 w-6 text-gray-700" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7" /></svg>
            </button>
            <h1 id="detail-title" class="text-2xl font-bold text-gray-800"></h1>
        </header>
        <div class="bg-gray-50 p-3 rounded-lg flex justify-center space-x-2 mb-6 mx-4">
            <button class="time-range-btn" data-range="hourly">Hourly</button>
            <button class="time-range-btn active" data-range="daily">Daily</button>
            <button class="time-range-btn" data-range="monthly">Monthly</button>
            <button class="time-range-btn" data-range="yearly">Yearly</button>
        </div>
        <div class="mb-6">
             <div class="w-full text-right text-sm text-gray-500 pr-4">kWh</div>
             <div class="relative h-64">
                <canvas id="detailChart"></canvas>
            </div>
        </div>
        <div class="px-4">
            <h2 id="list-heading" class="text-xl font-bold text-gray-800 text-left mb-4"></h2>
            <div id="detail-list" class="space-y-3"></div>
        </div>
    </div>

    <div class="bottom-nav">
        <button class="nav-btn" onclick="window.location.href='/'"> <i class="fa-solid fa-house"></i></button>
        <button class="nav-btn" onclick="window.location.href='/power'"> <i class="fa-solid fa-bolt"></i></button>
        <button class="nav-btn" style="color: #3e67bf"><i class="fa-solid fa-chart-column"></i></button>
        <button class="nav-btn" onclick="window.location.href='/settings'"> <i class="fa-solid fa-gear"></i></button>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-datalabels@2.2.0/dist/chartjs-plugin-datalabels.min.js"></script>
    <script>
        document.addEventListener('DOMContentLoaded', function () {
            const ESP_HOSTNAME = window.location.hostname;
            const ROOMS_KEY = 'smartHomeRooms';
            const CHART_COLORS = ['#3B82F6', '#10B981', '#F59E0B', '#8B5CF6', '#EF4444', '#6B7280', '#F97316'];
            const mainDashboard = document.getElementById('main-dashboard');
            const detailView = document.getElementById('detail-view');
            let detailChartInstance = null;
            let mainChartInstance = null;
            let localRoomsData = [];
            
            async function apiGet(endpoint) {
                try {
                    const response = await fetch(`http://${ESP_HOSTNAME}/${endpoint}`);
                    if (!response.ok) throw new Error(`HTTP error! status: ${response.status}`);
                    return await response.json();
                } catch (error) {
                    console.error(`Could not fetch from ${endpoint}.`, error);
                    return null;
                }
            }

            async function initMainDashboard() {
                localRoomsData = JSON.parse(localStorage.getItem(ROOMS_KEY)) || [];
                const espData = await apiGet('graph/today');
                if (!espData) return;

                const mergedRooms = espData.rooms.map(espRoom => {
                    const localRoom = localRoomsData.find(lr => lr.id === espRoom.id);
                    return { ...espRoom, name: localRoom ? localRoom.name : `Room ${espRoom.id}` };
                });

                const totalConsumptionEl = document.getElementById('totalConsumption');
                totalConsumptionEl.innerHTML = `<span>${espData.totalConsumption.toFixed(1)} kWh</span><svg xmlns="http://www.w3.org/2000/svg" class="h-6 w-6 ml-1 text-gray-300 group-hover:text-gray-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M9 5l7 7-7 7" /></svg>`;
                
                const ctx = document.getElementById('consumptionChart').getContext('2d');
                if(mainChartInstance) mainChartInstance.destroy();
                mainChartInstance = new Chart(ctx, { 
                    type: 'doughnut', 
                    data: { 
                        labels: mergedRooms.map(r => r.name), 
                        datasets: [{ 
                            data: mergedRooms.map(r => r.consumption), 
                            backgroundColor: mergedRooms.map((r, i) => CHART_COLORS[i % CHART_COLORS.length]),
                            borderWidth: 0 
                        }] 
                    }, 
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        cutout: '80%',
                        layout: {
                            padding: {
                                top: 30,
                                bottom: 30,
                                left: 30,
                                right: 30
                            }
                        },
                        plugins: {
                            // 1. Hide the old legend at the bottom
                            legend: {
                                display: false
                            },
                            tooltip: {
                                enabled: true
                            },
                            
                            // 2. Configure the new data labels
                            datalabels: {
                                // This function gets the label text (e.g., "Room 1")
                                formatter: (value, context) => {
                                    return context.chart.data.labels[context.dataIndex];
                                },
                                color: '#374151', // A dark gray color
                                anchor: 'end',    // Positions the anchor at the outer edge
                                align: 'end',   // Positions the label outside the anchor
                                offset: 8,      // Adds a small space from the chart
                                rotation: 0,    // <-- ADDED: Ensures text is always horizontal
                                font: {
                                    weight: '500',
                                    size: 11
                                },
                                // ADDED: Smartly aligns text to the left or right
                                textAlign: (context) => {
                                    const meta = context.chart.getDatasetMeta(context.datasetIndex);
                                    const slice = meta.data[context.dataIndex];
                                    if (!slice) return 'start';
                                    
                                    const midAngle = slice.startAngle + (slice.endAngle - slice.startAngle) / 2;
                                    
                                    // Normalize angle (0 is 3 o'clock)
                                    let normalizedAngle = midAngle % (2 * Math.PI);
                                    if (normalizedAngle < 0) {
                                        normalizedAngle += 2 * Math.PI;
                                    }

                                    // Right side of chart (270deg to 90deg)
                                    if (normalizedAngle > 1.5 * Math.PI || normalizedAngle < 0.5 * Math.PI) {
                                        return 'left'; // Text flows away to the right
                                    } else {
                                        // Left side of chart (90deg to 270deg)
                                        return 'right'; // Text flows away to the left
                                    }
                                }
                            }
                        }
                    },
                    plugins: [ChartDataLabels]
                });
                
                const roomDetailsContainer = document.getElementById('room-details');
                roomDetailsContainer.innerHTML = '';
                mergedRooms.forEach((room, i) => {
                    const card = document.createElement('a');
                    card.className = 'room-card group';
                    card.dataset.roomId = room.id;
                    card.dataset.roomName = room.name;
                    const color = CHART_COLORS[i % CHART_COLORS.length];
                    card.innerHTML = `<div class="flex items-center justify-start [width:-webkit-fill-available]"><div class="w-3 h-3 rounded-full mr-3" style="background-color: ${color};"></div><p class="text-gray-600 font-medium text-sm">${room.name}</p></div><div class="flex items-center"><p class="text-gray-800 font-bold text-lg">${room.consumption.toFixed(1)} kWh</p><svg xmlns="http://www.w3.org/2000/svg" class="h-5 w-5 ml-2 text-gray-300 group-hover:text-gray-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path stroke-linecap="round" stroke-linejoin="round" d="M9 5l7 7-7 7" /></svg></div>`;
                    roomDetailsContainer.appendChild(card);
                });
            }

            async function updateDetailView(roomId, range) {
                const data = await apiGet(`graph/details?roomId=${roomId}&range=${range}`);
                if (!data) return;

                const ctx = document.getElementById('detailChart').getContext('2d');
                if (detailChartInstance) detailChartInstance.destroy();
                detailChartInstance = new Chart(ctx, { type: 'bar', data: { labels: data.chart.labels, datasets: [{ label: 'kWh', data: data.chart.data, backgroundColor: '#60A5FA', borderRadius: 6, borderSkipped: false, }] }, options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { position: 'right', beginAtZero: true, grid: { drawBorder: false }, ticks: { padding: 10 } }, x: { grid: { display: false }, ticks: { padding: 10 } } } } });

                const detailList = document.getElementById('detail-list');
                const listHeading = document.getElementById('list-heading');
                detailList.innerHTML = '';
                
                if (roomId === 'Total') {
                    listHeading.textContent = 'Room Consumption';
                     const mergedList = data.list.map((espRoom, i) => {
                        const localRoom = localRoomsData.find(lr => lr.id === espRoom.id);
                        return { ...espRoom, name: localRoom ? localRoom.name : `Room ${espRoom.id}`, color: CHART_COLORS[i % CHART_COLORS.length]};
                    });
                    mergedList.forEach(item => {
                        const roomEl = document.createElement('div');
                        roomEl.className = 'flex justify-between items-center bg-white p-4 rounded-lg border';
                        roomEl.innerHTML = `<div class="flex items-center"><div class="w-3 h-3 rounded-full mr-3" style="background-color: ${item.color};"></div><p class="font-bold text-gray-700">${item.name}</p></div><p class="font-bold text-lg text-indigo-600">${item.consumption.toFixed(1)} kWh</p>`;
                        detailList.appendChild(roomEl);
                    });
                } else {
                    listHeading.textContent = 'Appliance Usage';
                     if (data.list && data.list.length > 0) {
                        data.list.forEach(item => {
                            const applianceEl = document.createElement('div');
                            applianceEl.className = 'flex justify-between items-center bg-white p-4 rounded-lg border';
                            applianceEl.innerHTML = `<div><p class="font-bold text-gray-700">${item.name}</p><p class="text-sm text-gray-500">Uptime: ${item.uptime}</p></div><p class="font-bold text-lg text-indigo-600">${item.consumption.toFixed(1)} kWh</p>`;
                            detailList.appendChild(applianceEl);
                        });
                    } else {
                        detailList.innerHTML = `<p class="text-gray-500">No appliance data available.</p>`;
                    }
                }
            }
            
            function showDetailView(roomId, roomName) {
                mainDashboard.classList.add('hidden');
                detailView.classList.remove('hidden');
                document.getElementById('detail-title').textContent = roomName;
                document.querySelectorAll('.time-range-btn').forEach(btn => btn.classList.toggle('active', btn.dataset.range === 'daily'));
                updateDetailView(roomId, 'daily');
            }

            document.body.addEventListener('click', e => { 
                const card = e.target.closest('[data-room-id]');
                if (card) {
                    showDetailView(card.dataset.roomId, card.dataset.roomName);
                }
            });
            document.getElementById('back-btn').addEventListener('click', () => { detailView.classList.add('hidden'); mainDashboard.classList.remove('hidden'); if (detailChartInstance) { detailChartInstance.destroy(); detailChartInstance = null; } });
            document.querySelectorAll('.time-range-btn').forEach(button => { button.addEventListener('click', e => { 
                document.querySelectorAll('.time-range-btn').forEach(btn => btn.classList.remove('active'));
                e.currentTarget.classList.add('active'); 
                const titleEl = document.getElementById('detail-title');
                const originalCard = document.querySelector(`[data-room-name="${titleEl.textContent}"]`);
                const roomId = originalCard ? originalCard.dataset.roomId : "Total";
                updateDetailView(roomId, e.currentTarget.dataset.range); 
            }); });
            
            initMainDashboard();
        });
    </script>
</body>
</html>
)RAW_HTML";
