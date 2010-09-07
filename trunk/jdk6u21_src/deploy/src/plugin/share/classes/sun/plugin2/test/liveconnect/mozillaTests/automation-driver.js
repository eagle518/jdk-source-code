// This driver file assumes the presence of an applet named "driver" on the page

function setHeader(header) {
    driver.setHeader(header);
}

function pass(message) {
    driver.pass(message);
}

function fail(message) {
    driver.fail(message);
}

function exception(message) {
    driver.exception(message);
}

function checkResults() {
    driver.checkResults();
}
