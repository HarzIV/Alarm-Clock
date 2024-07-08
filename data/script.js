document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('alarmForm');
    const list = document.getElementById('alarmList');

    function sendAlarms() {
        const alarms = [...list.children].map(item => item.textContent.trim());
        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/submit-alarms', true);
        xhr.setRequestHeader('Content-Type', 'application/json;charset=UTF-8');
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4 && xhr.status === 200) {
                console.log('Alarms submitted successfully');
            } else if (xhr.readyState === 4) {
                console.error('Failed to submit alarms');
            }
        };
        xhr.send(JSON.stringify({ alarms }));
    }

    form.addEventListener('submit', function(event) {
        event.preventDefault(); // Prevent form submission

        const hoursInput = document.getElementById('hours');
        const minutesInput = document.getElementById('minutes');
        const hours = parseInt(hoursInput.value);
        const minutes = parseInt(minutesInput.value);

        if (isNaN(hours) || isNaN(minutes)) {
            alert('Please enter valid hours and minutes.');
            return;
        }

        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
            alert('Please enter valid hours (0-23) and minutes (0-59).');
            return;
        }

        const alarmTime = ('0' + hours).slice(-2) + ':' + ('0' + minutes).slice(-2);
        addAlarmToList(alarmTime);

        // Clear input fields
        hoursInput.value = '';
        minutesInput.value = '';

        sendAlarms();
    });

    function addAlarmToList(alarmTime) {
        const listItem = document.createElement('li');
        listItem.textContent = alarmTime;

        const trashIcon = document.createElement('i');
        trashIcon.className = 'fas fa-trash-alt trash-icon';
        trashIcon.addEventListener('click', function(event) {
            listItem.remove();
            sendAlarms();
        });

        listItem.appendChild(trashIcon);
        list.appendChild(listItem);
    }

    function loadTimes() {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', '/getTimes', true); // Assuming the endpoint is '/getTimes'
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4 && xhr.status == 200) {
                var times = xhr.responseText.split(',');
                for (var i = 0; i < times.length; i++) {
                    if (times[i].trim() !== '') {
                        addAlarmToList(times[i].trim());
                    }
                }
            }
        };
        xhr.send();
    }

    window.onload = loadTimes;
});
