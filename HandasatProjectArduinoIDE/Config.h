#ifndef CONFIG_H
#define CONFIG_H

#define WiFiName "Omri"
#define WiFiPassword "12345678"

const char HTML_MAIN[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Tunables</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    table { border-collapse: collapse; margin-bottom: 24px; width: 400px; }
    th, td { border: 1px solid #999; padding: 8px; text-align: left; }
    th { background: #eee; }
    input { width: 120px; }
    button { padding: 6px 10px; }
    .group-title { margin-top: 24px; font-size: 20px; font-weight: bold; }
  </style>
</head>
<body>
  <h1>ESP32 Tunables</h1>
  <div id="tables"></div>

<script>
async function loadTunables() {
  const res = await fetch('/data');
  const data = await res.json();

  const root = document.getElementById('tables');
  root.innerHTML = '';

  data.forEach(group => {
    const title = document.createElement('div');
    title.className = 'group-title';
    title.textContent = group.group;
    root.appendChild(title);

    const table = document.createElement('table');

    const header = document.createElement('tr');
    header.innerHTML = '<th>Field</th><th>Value</th><th>Action</th>';
    table.appendChild(header);

    group.fields.forEach(field => {
      const row = document.createElement('tr');

      const nameCell = document.createElement('td');
      nameCell.textContent = field.name;

      const valueCell = document.createElement('td');
      const input = document.createElement('input');
      input.type = 'number';
      input.step = 'any';
      input.value = field.value;
      input.id = `${group.group}__${field.name}`;
      valueCell.appendChild(input);

      const actionCell = document.createElement('td');
      const btn = document.createElement('button');
      btn.textContent = 'Update';
      btn.onclick = async () => {
        const val = document.getElementById(`${group.group}__${field.name}`).value;
        await fetch(`/set?group=${encodeURIComponent(group.group)}&field=${encodeURIComponent(field.name)}&value=${encodeURIComponent(val)}`);
        await loadTunables();
      };
      actionCell.appendChild(btn);

      row.appendChild(nameCell);
      row.appendChild(valueCell);
      row.appendChild(actionCell);

      table.appendChild(row);
    });

    root.appendChild(table);
  });
}

loadTunables();
// setInterval(loadTunables, 10000);
</script>
</body>
</html>
)rawliteral";

#endif
