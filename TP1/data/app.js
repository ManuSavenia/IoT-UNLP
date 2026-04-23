function update() {
  fetch('/data')
    .then(r => r.json())
    .then(d => {
      document.getElementById('tv').innerText = d.t + '°C';
      document.getElementById('hv').innerText = d.h + '%';
      document.getElementById('wn').innerText = d.w;
      document.getElementById('ts').innerText = d.to ? 'CONECTADO' : 'ERROR';
      document.getElementById('hs').innerText = d.ho ? 'CONECTADO' : 'ERROR';
    })
    .catch(() => {
      ['tv', 'hv', 'wn', 'ts', 'hs'].forEach(id =>
        document.getElementById(id).innerText = '...'
      );
    });
}

setInterval(update, 2000);
update();