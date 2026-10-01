using CUE4Parse.FileProvider;
using CUE4Parse.Compression;
using CUE4Parse.Encryption.Aes;
using CUE4Parse.UE4.Objects.Core.Misc;
using CUE4Parse.UE4.Assets.Exports.Material;
using CUE4Parse.UE4.Assets.Exports.SkeletalMesh;
using CUE4Parse.UE4.Assets.Exports.Texture;
using CUE4Parse.UE4.Versions;
using CUE4Parse_Conversion;
using CUE4Parse_Conversion.Options;
using CUE4Parse_Conversion.Writers.UEFormat.Enums;

const string pakDirectory = @"D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Content\Paks";
const string outputDirectory = @"D:\codex\item scanner\model\reference\object-scanner-extracted";

OodleHelper.Initialize();
var provider = new DefaultFileProvider(
    pakDirectory,
    SearchOption.TopDirectoryOnly,
    true,
    new VersionContainer(EGame.GAME_UE5_6));
provider.Initialize();
provider.SubmitKey(new FGuid(), new FAesKey("0x0A45EC8C0289ADF8CB28473CF3BF5F4D24BB16D82BAD6562F2F9FD2B2DF98251"));

var options = new ExportOptions(
    EMeshFormat.ActorX,
    ENaniteMeshFormat.NoNanite,
    EMeshQuality.Highest,
    ETexturePlatform.DesktopMobile,
    ETextureFormat.Png,
    100,
    false,
    false,
    EMaterialDepth.TopLayerOnly,
    false,
    false,
    ESocketFormat.None,
    EFileCompressionFormat.None);

Directory.CreateDirectory(outputDirectory);
var matchingKeys = provider.Files.Keys
    .Where(key => key.Contains("ObjectScanner", StringComparison.OrdinalIgnoreCase)
        || key.Contains("Object_Scanner", StringComparison.OrdinalIgnoreCase))
    .OrderBy(key => key)
    .ToArray();
File.WriteAllLines(Path.Combine(outputDirectory, "object_scanner_paths.txt"), matchingKeys);
Console.WriteLine($"FILES {provider.Files.Count}; MATCHES {matchingKeys.Length}");
foreach (var key in matchingKeys) Console.WriteLine("FILE " + key);
Console.Out.Flush();
if (matchingKeys.Length == 0) throw new InvalidOperationException("No Object Scanner files mounted");

const string objectScannerPath =
    "FactoryGame/Content/FactoryGame/Equipment/ObjectScanner/Mesh/SK_ObjectScanner_01.SK_ObjectScanner_01";
var mesh = provider.LoadPackageObject<USkeletalMesh>(objectScannerPath);
Console.WriteLine($"LOADED {objectScannerPath}: {mesh.Name}");
var session = new ExportSession((_, _) => { });
session.Add(mesh);
var results = await session.RunAsync(outputDirectory, options, null, CancellationToken.None);
foreach (var result in results)
    Console.WriteLine(result);
