const char HTML_BODY[] =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<title>Estação</title>"

    // Estilos CSS para a página e seus componentes
    "<style>"
    "body{font-family:sans-serif;text-align:center;margin:0;padding:20px;background:#f2f2f2;color:#333}"  // Estilo geral do corpo da página
    "h1{margin-bottom:10px}"                                                                         // Espaçamento do título
    ".card{background:#fff;padding:20px;margin-bottom:20px;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.1);max-width:900px;margin:0 auto}"  // Cartão branco centralizado com sombra
    ".valores{display:flex;flex-wrap:wrap;justify-content:center;gap:20px;margin-bottom:10px}"       // Container flexível para valores com espaçamento
    ".valor-box{background:#fafafa;padding:15px;border-radius:10px;box-shadow:inset 0 1px 3px rgba(0,0,0,0.1);flex:1 1 120px;}" // Caixa individual para cada valor
    ".valor-box h3{margin:0;font-size:16px;color:#555}"                                              // Títulos dos valores
    ".valor-box span{font-size:18px;font-weight:bold;color:#000}"                                    // Valores em destaque
    ".valor-box small{font-size:12px;color:#666;}"                                                  // Texto auxiliar menor
    ".botoes-topo{margin-top:15px}"                                                                // Espaço acima dos botões
    ".botao{font-size:16px;padding:10px 20px;margin:5px;border:none;border-radius:8px;cursor:pointer}" // Estilo dos botões
    ".on{background:#4CAF50;color:#fff}"                                                            // Classe para botão verde (ligado)
    ".off{background:#f44336;color:#fff}"                                                           // Classe para botão vermelho (desligado)
    ".grid{display:flex;flex-wrap:wrap;justify-content:center;gap:20px;max-width:1200px;margin:20px auto}" // Layout flexível para gráficos
    ".grafico-container{flex:1 1 45%;min-width:300px;padding:15px;background:#fff;border-radius:12px;box-shadow:0 2px 5px rgba(0,0,0,0.1)}" // Container individual dos gráficos
    "canvas{width:100%;height:auto}"                                                                // Canvas responsivo
    ".modal{display:none;position:fixed;z-index:999;left:0;top:0;width:100%;height:100%;overflow:auto;background:rgba(0,0,0,0.5)}" // Fundo do modal escurecido oculto por padrão
    ".modal-conteudo{background:#fff;margin:5% auto;padding:25px;border-radius:14px;width:90%;max-width:500px;box-shadow:0 6px 20px rgba(0,0,0,0.3)}" // Conteúdo modal centralizado
    ".fechar{color:#999;float:right;font-size:28px;font-weight:bold;cursor:pointer;margin-top:-10px}" // Botão fechar modal no canto superior direito
    ".fechar:hover,.fechar:focus{color:#000;text-decoration:none}"                                  // Efeito hover/focus do fechar
    ".modal-conteudo h3{text-align:center;margin-bottom:20px;color:#333}"                           // Título dentro do modal
    ".modal-grid{display:grid;grid-template-columns:1fr 1fr;gap:15px;margin-bottom:15px}"           // Grid para inputs dentro do modal
    ".modal-conteudo form,.modal-grid div{display:flex;flex-direction:column;align-items:stretch}" // Inputs e botões dentro do modal alinhados verticalmente
    ".modal-conteudo label{font-weight:bold;margin-bottom:4px;font-size:14px}"                      // Labels dos inputs
    ".modal-conteudo input{padding:8px;border:1px solid #ccc;border-radius:6px;font-size:15px;margin-bottom:4px}" // Inputs estilizados
    ".modal-conteudo .botao{padding:8px;font-size:15px}"                                            // Botões dentro do modal
    ".mensagem-final{margin-top:20px;padding:10px;background:#d4edda;color:#155724;border-radius:6px;font-weight:bold;display:none}" // Mensagem de sucesso padrão oculta inicialmente
    ".mensagem-final.erro{background:#f8d7da;color:#721c24}"                                        // Estilo para mensagem de erro
    "@media(max-width:1000px){.grid{flex-direction:column;align-items:center}.grafico-container{width:90%}.botao{width:90%}}" // Responsividade para telas menores que 1000px
    "@media(max-width:600px){.modal-grid{grid-template-columns:1fr}}"                               // Responsividade para modal em telas muito pequenas
    "</style>"

    // Inclusão da biblioteca Chart.js para gráficos
    "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"

    // Script JavaScript para controlar gráficos e interações
    "<script>"
    "let g1,g2,g3,g4,d={x:[],y:[],z:[],t:[]},n=30;" // Variáveis globais para gráficos e dados acumulados (x=temp, y=altitude, z=umidade, t=tempo), n=limite de pontos

    // Função para criar gráfico de linha simples
    "function cg(i,l,c){"
    "return new Chart(document.getElementById(i).getContext('2d'),{"
    "type:'line',"
    "data:{labels:[],datasets:[{label:l,data:[],borderColor:c,fill:false}]},"
    "options:{responsive:true,maintainAspectRatio:true,aspectRatio:1.3,animation:false,"
    "scales:{x:{title:{display:true,text:'Tempo'}},y:{title:{display:true,text:'Valor'}}}}"
    "});"
    "}"

    // Função para criar gráfico de linhas múltiplas (para combinado)
    "function cgmulti(i){"
    "return new Chart(document.getElementById(i).getContext('2d'),{"
    "type:'line',"
    "data:{labels:[],datasets:["
    "{label:'Temp (°C)',data:[],borderColor:'#2196F3',fill:false},"
    "{label:'Pressão (hPa)',data:[],borderColor:'#B16099',fill:false},"
    "{label:'Umidade (%)',data:[],borderColor:'#FF9800',fill:false}"
    "]},"
    "options:{responsive:true,maintainAspectRatio:true,aspectRatio:1.3,animation:false,"
    "scales:{x:{title:{display:true,text:'Tempo'}},y:{title:{display:true,text:'Valor'}}}}"
    "});"
    "}"

    // Inicialização após carregamento da página
    "document.addEventListener('DOMContentLoaded',()=>{"
    "g1=cg('g1','Temp','#2196F3');"     // Gráfico temperatura
    "g2=cg('g2','Pressão','#B16099');"  // Gráfico pressão
    "g3=cg('g3','Umidade','#FF9800');"  // Gráfico umidade
    "g4=cgmulti('g4');"                 // Gráfico combinado

    // Evento para enviar offset ao apertar Enter nos inputs
    "document.querySelectorAll('.offset-input').forEach(i=>{"
    "i.addEventListener('keypress',e=>{"
    "if(e.key==='Enter'){e.preventDefault();enviarOffset(i.id.replace('offset_',''));}"
    "});"
    "});"

    // Eventos para salvar limites ao apertar Enter nos inputs correspondentes
    "['temp','press','umid'].forEach(tipo=>{"
    "['min_','max_'].forEach(prefix=>{"
    "let input=document.getElementById(prefix+tipo);"
    "input.addEventListener('keypress',e=>{"
    "if(e.key==='Enter'){e.preventDefault();salvarLimite(tipo);}"
    "});"
    "});"
    "});"
    "});"

    // Função para atualizar dados e gráficos a cada segundo
    "function att(){"
    "fetch('/estado').then(r=>r.json()).then(e=>{"
    "let t=new Date().toLocaleTimeString();" // Tempo atual formatado
    // Remove dados antigos se exceder limite n
    "if(d.x.length>=n){d.x.shift();d.y.shift();d.z.shift();d.t.shift();}"

    // Adiciona novos dados recebidos do servidor
    "d.x.push(e.x); d.y.push(e.y); d.z.push(e.z); d.t.push(t);"

    // Atualiza valores exibidos na página
    "document.getElementById('v_temp').innerText = e.x + ' °C';"
    "document.getElementById('v_pressao').innerText = e.y + ' hPa';"
    "document.getElementById('v_umidade').innerText = e.z + ' %';"

    // Atualiza exibição dos offsets e limites
    "document.getElementById('offset_temp_disp').innerText = e.offset_temp ?? '--';"
    "document.getElementById('min_temp_disp').innerText = e.min_temp ?? '--';"
    "document.getElementById('max_temp_disp').innerText = e.max_temp ?? '--';"

    "document.getElementById('offset_pressao_disp').innerText = e.offset_pressao ?? '--';"
    "document.getElementById('min_press_disp').innerText = e.min_press ?? '--';"
    "document.getElementById('max_press_disp').innerText = e.max_press ?? '--';"

    "document.getElementById('offset_umidade_disp').innerText = e.offset_umidade ?? '--';"
    "document.getElementById('min_umid_disp').innerText = e.min_umid ?? '--';"
    "document.getElementById('max_umid_disp').innerText = e.max_umid ?? '--';"

    // Atualiza os dados dos gráficos se definidos
    "if(g1){g1.data.labels=d.t;g1.data.datasets[0].data=d.x;g1.update();}"
    "if(g2){g2.data.labels=d.t;g2.data.datasets[0].data=d.y;g2.update();}"
    "if(g3){g3.data.labels=d.t;g3.data.datasets[0].data=d.z;g3.update();}"
    "if(g4){g4.data.labels=d.t;g4.data.datasets[0].data=d.x;g4.data.datasets[1].data=d.y;g4.data.datasets[2].data=d.z;g4.update();}"
    "});"
    "}"

    // Funções para abrir e fechar modais
    "function abrirModal(){document.getElementById('modal').style.display='block'}"
    "function fecharModal(){document.getElementById('modal').style.display='none'}"
    "function abrirLimites(){document.getElementById('limites').style.display='block'}"
    "function fecharLimites(){document.getElementById('limites').style.display='none'}"

    // Envia offset para o servidor via fetch
    "function enviarOffset(t){"
    "const i=document.getElementById('offset_'+t),v=i.value.trim(),m=document.getElementById('mensagem_final');"
    "if(v===''){"
    "m.textContent='Preencha o valor para aplicar o offset';"
    "m.className='mensagem-final erro';"
    "m.style.display='block';"
    "setTimeout(()=>{m.style.display='none';},3000);"
    "return;"
    "}"
    "fetch('/offset/'+t+'/'+encodeURIComponent(v)).then(()=>{"
    "i.value='';"
    "let txt=t==='temp'?'Temperatura':t==='pressao'?'Pressão':'Umidade';"
    "m.textContent=txt+' calibrada com sucesso!';"
    "m.className='mensagem-final';"
    "m.style.display='block';"
    "setTimeout(()=>{m.style.display='none';},3000);"
    "});"
    "}"

    // Salva limites mínimos e máximos para os tipos
    "function salvarLimite(tipo){"
    "let minInput=document.getElementById('min_'+tipo),"
    "maxInput=document.getElementById('max_'+tipo),"
    "min=minInput.value.trim(),"
    "max=maxInput.value.trim(),"
    "m=document.getElementById('mensagem_limites');"

    "if(min===''||max===''){"
    "m.textContent='Preencha os dois valores de '+tipo;"
    "mostrarErro();"
    "return;"
    "}"
    "if(parseFloat(min)>parseFloat(max)){"
    "m.textContent='Mínimo não pode ser maior que o máximo';"
    "mostrarErro();"
    "return;"
    "}"

    "fetch('/limites/'+tipo+'/min/'+min+'/max/'+max).then(()=>{"
    "m.textContent='Limites de '+tipo+' salvos com sucesso!';"
    "m.className='mensagem-final';"
    "m.style.display='block';"
    "setTimeout(()=>{m.style.display='none';},3000);"
    "minInput.value='';"
    "maxInput.value='';"
    "});"
    "}"

    // Mostra mensagem de erro em limites
    "function mostrarErro(){"
    "let m=document.getElementById('mensagem_limites');"
    "m.className='mensagem-final erro';"
    "m.style.display='block';"
    "setTimeout(()=>{m.style.display='none';},4000);"
    "}"

    "function val(id){return document.getElementById(id).value}" // Função utilitária para obter valor input

    // Atualiza dados a cada 1 segundo
    "setInterval(att,1000);"
    "</script>"
    "</head>"

    "<body>"
    "<h1>Estação Meteorológica</h1>"

    // Cartão com valores atuais e botões para abrir modais
    "<div class='card'>"
    "<div class='valores'>"
    "<div class='valor-box'>"
    "<h3>Temperatura</h3>"
    "<span id='v_temp'>--</span><br>"
    "<small>Offset: <span id='offset_temp_disp'>--</span></small><br>"
    "<small>Min: <span id='min_temp_disp'>--</span> | Max: <span id='max_temp_disp'>--</span></small>"
    "</div>"
    "<div class='valor-box'>"
    "<h3>Pressão</h3>"
    "<span id='v_pressao'>--</span><br>"
    "<small>Offset: <span id='offset_pressao_disp'>--</span></small><br>"
    "<small>Min: <span id='min_press_disp'>--</span> | Max: <span id='max_press_disp'>--</span></small>"
    "</div>"
    "<div class='valor-box'>"
    "<h3>Umidade</h3>"
    "<span id='v_umidade'>--</span><br>"
    "<small>Offset: <span id='offset_umidade_disp'>--</span></small><br>"
    "<small>Min: <span id='min_umid_disp'>--</span> | Max: <span id='max_umid_disp'>--</span></small>"
    "</div>"
    "</div>"

    // Botões para abrir modais de offset e limites
    "<div class='botoes-topo'>"
    "<button class='botao on' onclick='abrirModal()'>Offsets</button>"
    "<button class='botao off' onclick='abrirLimites()'>Limites</button>"
    "</div>"
    "</div>"

    // Container para os gráficos
    "<div class='grid'>"
    "<div class='grafico-container'><h3>Temperatura</h3><canvas id='g1'></canvas></div>"
    "<div class='grafico-container'><h3>Pressão</h3><canvas id='g2'></canvas></div>"
    "<div class='grafico-container'><h3>Umidade</h3><canvas id='g3'></canvas></div>"
    "<div class='grafico-container'><h3>Combinado</h3><canvas id='g4'></canvas></div>"
    "</div>"

    // Modal para configuração de offsets
    "<div id='modal' class='modal'>"
    "<div class='modal-conteudo'>"
    "<span class='fechar' onclick='fecharModal()'>&times;</span>"
    "<h3>Configuração de Offsets</h3>"
    "<div class='modal-grid'>"
    "<div><label>Temperatura:</label><input type='number' class='offset-input' id='offset_temp' step='any'><button type='button' class='botao on' onclick=\"enviarOffset('temp')\">Aplicar</button></div>"
    "<div><label>Pressão:</label><input type='number' class='offset-input' id='offset_pressao' step='any'><button type='button' class='botao on' onclick=\"enviarOffset('pressao')\">Aplicar</button></div>"
    "<div><label>Umidade:</label><input type='number' class='offset-input' id='offset_umidade' step='any'><button type='button' class='botao on' onclick=\"enviarOffset('umidade')\">Aplicar</button></div>"
    "</div>"
    "<div id='mensagem_final' class='mensagem-final'></div>"  // Mensagem de sucesso ou erro do offset
    "</div>"
    "</div>"

    // Modal para configuração de limites mínimos e máximos
    "<div id='limites' class='modal'>"
    "<div class='modal-conteudo'>"
    "<span class='fechar' onclick='fecharLimites()'>&times;</span>"
    "<h3>Definir Limites</h3>"
    "<div class='modal-grid'>"
    "<div><label>Temp Min:</label><input type='number' id='min_temp' step='any'><label>Temp Max:</label><input type='number' id='max_temp' step='any'><button type='button' class='botao on' onclick=\"salvarLimite('temp')\">Salvar</button></div>"
    "<div><label>Pres Min:</label><input type='number' id='min_press' step='any'><label>Pres Max:</label><input type='number' id='max_press' step='any'><button type='button' class='botao on' onclick=\"salvarLimite('press')\">Salvar</button></div>"
    "<div><label>Umid Min:</label><input type='number' id='min_umid' step='any'><label>Umid Max:</label><input type='number' id='max_umid' step='any'><button type='button' class='botao on' onclick=\"salvarLimite('umid')\">Salvar</button></div>"
    "</div>"
    "<div id='mensagem_limites' class='mensagem-final'></div>"  // Mensagem de sucesso ou erro dos limites
    "</div>"
    "</div>"

    // Rodapé da página
    "<footer style='margin-top:40px;font-size:14px;color:#666'>"
    "<hr>"
    "<p><em>Embarcatech - Sistema de monitoramento metereológico</em></p>"
    "</footer>"
    "</body>"
    "</html>";
