# Vider la corbeille sous Windows
$Shell = New-Object -ComObject Shell.Application
$Shell.Namespace(10).Items() | ForEach-Object { $_.InvokeVerb("delete") }
Write-Output "Corbeille vidée avec succès."
