module.exports = {
    flowFile: 'flows.json',
    credentialSecret: false,   // credenciales en texto plano en flows_cred.json
    userDir: '/data/',
    nodesDir: '/data/nodes/',
    logging: {
        console: {
            level: "info",
            metrics: false,
            audit: false
        }
    },
    editorTheme: {
        page: { title: "IoT - Monitoreo Ambiental" }
    }
}
