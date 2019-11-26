function firebase() {
    var firebaseConfig = {
        apiKey: "AIzaSyCJmL9p_Fq7WU-H8r9kOJWJZBBjDTI8bOs",
        authDomain: "testestcc-38d19.firebaseapp.com",
        databaseURL: "https://testestcc-38d19.firebaseio.com",
        projectId: "testestcc-38d19",
        storageBucket: "testestcc-38d19.appspot.com"
    };
    firebase.initializeApp(firebaseConfig);
    var database = firebase.database();
    var temp = database.ref('Temperatura');
    temp.on('value', function (snapshot) {
        renderChart(snapshot.val());
    });
}

function renderChart(data) {
    var ctx = "myChart";
    var myChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: "Temperatura",
            datasets: [{
                label: 'This week',
                data: data,
            }]
        },
    });
};

function addData(chart, label, data) {
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset) => {
        dataset.data.push(data);
    });
    chart.update();
}

window.onload = function() {
    firebase();
};