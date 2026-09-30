using System;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;

[assembly: AssemblyTitle("IntelBurnTest USB Setup")]
[assembly: AssemblyProduct("IntelBurnTest")]
[assembly: AssemblyVersion("3.2.0.0")]
[assembly: AssemblyFileVersion("3.2.0.0")]
[assembly: TargetFramework(".NETFramework,Version=v4.8")]

namespace IntelBurnTestUsb
{
    internal static class Payload
    {
        static byte[] Resource(string name, string expected)
        {
            using (Stream stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                if (stream == null) throw new IOException("The installer is incomplete. Download it again from the official release.");
                using (MemoryStream memory = new MemoryStream())
                {
                    stream.CopyTo(memory); byte[] bytes = memory.ToArray();
                    if (Safety.Hash(bytes) != expected) throw new IOException("The test files did not verify. Download the installer again.");
                    return bytes;
                }
            }
        }
        internal static byte[] Efi() { return Resource("IBT.EFI", BuildPayload.EfiHash); }
        internal static byte[] Readme() { return Resource("IBT.README", BuildPayload.ReadmeHash); }
    }

    internal sealed class ConfirmForm : Form
    {
        internal readonly TextBox Consent = new TextBox { Dock = DockStyle.Top };
        internal readonly Button Install = new Button { AutoSize = true, MinimumSize = new Size(160, 38) };
        internal ConfirmForm(InstallPlan plan)
        {
            Text = plan.Erase ? "Confirm USB erase" : "Confirm USB installation";
            Font = new Font("Segoe UI", 10); StartPosition = FormStartPosition.CenterParent;
            FormBorderStyle = FormBorderStyle.FixedDialog; MaximizeBox = MinimizeBox = false;
            ClientSize = new Size(580, plan.Erase ? 415 : 335); AutoScaleMode = AutoScaleMode.Dpi;
            TableLayoutPanel content = new TableLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(24), ColumnCount = 1, RowCount = 5 };
            content.RowStyles.Add(new RowStyle(SizeType.AutoSize)); content.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            content.RowStyles.Add(new RowStyle(SizeType.Percent, 100)); content.RowStyles.Add(new RowStyle(SizeType.AutoSize)); content.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            string title = plan.Erase ? "All files on this USB will be deleted." : "Install IntelBurnTest on this USB?";
            content.Controls.Add(new Label { Text = title, AutoSize = true, MaximumSize = new Size(525, 0), Font = new Font(Font, FontStyle.Bold),
                ForeColor = plan.Erase ? Color.Firebrick : Color.FromArgb(170, 65, 12), Margin = new Padding(0, 0, 0, 16) });
            string volumes = plan.Disk.Volumes.Count == 0 ? "No mounted volumes" : String.Join("\n", plan.Disk.Volumes.Select(v => v.ToString()));
            content.Controls.Add(new Label { Text = plan.Disk + "\n" + (plan.Erase ? "Affected volumes:\n" + volumes : "Destination: " + plan.Volume),
                AutoSize = true, MaximumSize = new Size(525, 0), Margin = new Padding(0, 0, 0, 12) });
            content.Controls.Add(new Label { Text = plan.Erase ? "Back up anything you want to keep before continuing. Keep the USB connected until setup finishes." :
                "Your other files will be kept. If another boot file exists, it will be backed up before being replaced. Keep the USB connected until setup finishes.",
                AutoSize = true, MaximumSize = new Size(525, 0) });
            if (plan.Erase)
            {
                FlowLayoutPanel entry = new FlowLayoutPanel { AutoSize = true, Dock = DockStyle.Fill, FlowDirection = FlowDirection.TopDown, Margin = new Padding(0, 12, 0, 12) };
                entry.Controls.Add(new Label { AutoSize = true, Text = "Type ERASE to confirm:" }); Consent.Width = 220;
                entry.Controls.Add(Consent); content.Controls.Add(entry, 0, 3);
            }
            else content.Controls.Add(new Label { AutoSize = true, Text = "" }, 0, 3);
            FlowLayoutPanel actions = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, FlowDirection = FlowDirection.RightToLeft };
            Button cancel = new Button { Text = "Cancel", DialogResult = DialogResult.Cancel, AutoSize = true, MinimumSize = new Size(100, 38) };
            Install.Text = plan.Erase ? "Erase and install" : "Install"; Install.Enabled = !plan.Erase; Install.DialogResult = DialogResult.OK;
            Consent.TextChanged += delegate { Install.Enabled = Safety.EraseConsent(Consent.Text); };
            actions.Controls.Add(cancel); actions.Controls.Add(Install); content.Controls.Add(actions, 0, 4);
            Controls.Add(content); CancelButton = cancel; AcceptButton = cancel; // Enter never silently authorizes an erase.
        }
    }

    internal sealed class SetupForm : Form
    {
        readonly IUsbProbe probe;
        readonly ComboBox devices = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList, Dock = DockStyle.Fill };
        readonly ComboBox volumes = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList, Dock = DockStyle.Fill };
        readonly Button refresh = new Button { Text = "Refresh", AutoSize = true };
        readonly CheckBox erase = new CheckBox { Text = "Format this USB and erase all its files", AutoSize = true, Margin = new Padding(0, 16, 0, 10) };
        readonly Button prepare = new Button { Text = "Prepare USB", MinimumSize = new Size(160, 42), AutoSize = true, Enabled = false };
        readonly ProgressBar progress = new ProgressBar { Dock = DockStyle.Fill, Height = 20, Visible = false };
        readonly Label detail = new Label { AutoSize = true, MaximumSize = new Size(610, 0), ForeColor = Color.FromArgb(80, 80, 80), Margin = new Padding(0, 8, 0, 16) };
        readonly Label status = new Label { AutoSize = true, MaximumSize = new Size(610, 0), Margin = new Padding(0, 12, 0, 16) };
        bool busy;
        internal SetupForm(IUsbProbe probe, bool loadOnShow)
        {
            this.probe = probe; Text = "IntelBurnTest 3.2 — USB Setup";
            Font = new Font("Segoe UI", 10); ClientSize = new Size(660, 510); MinimumSize = new Size(676, 549);
            StartPosition = FormStartPosition.CenterScreen; AutoScaleMode = AutoScaleMode.Dpi;
            BackColor = Color.White;
            TableLayoutPanel page = new TableLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(26), ColumnCount = 1, AutoScroll = true };
            page.Controls.Add(new Label { Text = "IntelBurnTest", Font = new Font("Segoe UI", 23, FontStyle.Bold), ForeColor = Color.FromArgb(199, 77, 17), AutoSize = true, Margin = new Padding(0, 0, 0, 2) });
            page.Controls.Add(new Label { Text = "Prepare a USB. Restart. Test your PC.", AutoSize = true, Margin = new Padding(0, 0, 0, 22) });
            page.Controls.Add(new Label { Text = "Choose your USB", AutoSize = true, Font = new Font(Font, FontStyle.Bold), Margin = new Padding(0, 0, 0, 8) });
            TableLayoutPanel picker = new TableLayoutPanel { Dock = DockStyle.Top, AutoSize = true, ColumnCount = 2 };
            picker.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100)); picker.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
            devices.Margin = new Padding(0, 3, 12, 3); picker.Controls.Add(devices); picker.Controls.Add(refresh); page.Controls.Add(picker);
            page.Controls.Add(volumes); page.Controls.Add(erase); page.Controls.Add(detail);
            page.Controls.Add(prepare); page.Controls.Add(progress); page.Controls.Add(status);
            page.Controls.Add(new Label { Text = "For PCs with 64-bit UEFI. Secure Boot must be disabled for this unsigned release.", AutoSize = true,
                MaximumSize = new Size(610, 0), ForeColor = Color.FromArgb(95, 95, 95), Margin = new Padding(0, 4, 0, 0) });
            Controls.Add(page);
            devices.SelectedIndexChanged += delegate { DiskChanged(); };
            erase.CheckedChanged += delegate { UpdateChoice(); };
            volumes.SelectedIndexChanged += delegate { UpdateChoice(); };
            refresh.Click += async delegate { await RefreshDevices(); };
            prepare.Click += async delegate { await Prepare(); };
            FormClosing += delegate(object sender, FormClosingEventArgs e) { if (busy) { e.Cancel = true; status.Text = "Please wait until setup finishes before closing."; } };
            if (loadOnShow) Shown += async delegate { await RefreshDevices(); };
            else status.Text = "Select the USB you want to prepare.";
        }
        void SetBusy(bool value)
        { busy = value; devices.Enabled = refresh.Enabled = erase.Enabled = volumes.Enabled = !value; prepare.Enabled = !value && devices.SelectedItem != null; }
        async Task RefreshDevices()
        {
            SetBusy(true); prepare.Enabled = false; status.Text = "Looking for USB drives..."; progress.Visible = false;
            try
            {
                var list = await Task.Run(() => probe.List()); devices.Items.Clear();
                devices.Items.AddRange(list.Cast<object>().ToArray()); devices.SelectedIndex = -1; volumes.Visible = false;
                status.ForeColor = Color.FromArgb(45, 45, 45);
                status.Text = list.Count == 0 ? "No eligible USB found. Connect a writable USB, then click Refresh." : "Select the USB you want to prepare.";
                detail.Text = "System disks and internal drives are excluded.";
            }
            catch (Exception e) { status.ForeColor = Color.Firebrick; status.Text = "Cannot inspect USB drives. Close setup and try again.\n" + e.Message; }
            finally { SetBusy(false); UpdateChoice(); }
        }
        void DiskChanged()
        {
            UsbDisk disk = devices.SelectedItem as UsbDisk; volumes.Items.Clear(); erase.Checked = false;
            if (disk != null) volumes.Items.AddRange(disk.Volumes.Where(Safety.Copyable).Cast<object>().ToArray());
            if (volumes.Items.Count > 0) volumes.SelectedIndex = 0;
            volumes.Visible = volumes.Items.Count > 1; UpdateChoice();
            progress.Visible = false; status.Text = "";
        }
        void UpdateChoice()
        {
            if (busy) return;
            UsbDisk disk = devices.SelectedItem as UsbDisk;
            bool copy = volumes.SelectedItem != null; bool allowed = disk != null && (copy || erase.Checked);
            if (disk != null && erase.Checked)
            {
                detail.ForeColor = Color.Firebrick;
                detail.Text = "All partitions and files on this USB will be deleted. The drive will be prepared as FAT32 using its full capacity.";
                try { Fat32Layout.Create(disk.Size, disk.SectorSize); }
                catch (IOException e) { detail.Text = e.Message; allowed = false; }
            }
            else
            {
                detail.ForeColor = Color.FromArgb(80, 80, 80);
                detail.Text = disk == null ? "System disks and internal drives are excluded." : copy ?
                    "FAT32 detected. Your files will be kept; an existing boot file will be backed up." :
                    "This USB needs FAT32. Back up your files, then select the format option to continue.";
            }
            prepare.Text = erase.Checked ? "Format and install..." : "Prepare USB...";
            prepare.Enabled = allowed; volumes.Enabled = !erase.Checked;
        }
        async Task Prepare()
        {
            UsbDisk disk = devices.SelectedItem as UsbDisk; if (disk == null) return;
            try
            {
                InstallPlan plan = new InstallPlan(disk, volumes.SelectedItem as UsbVolume, erase.Checked);
                using (ConfirmForm confirmation = new ConfirmForm(plan))
                    if (confirmation.ShowDialog(this) != DialogResult.OK) return;
                SetBusy(true); prepare.Enabled = false; progress.Value = 0; progress.Visible = true;
                status.ForeColor = Color.FromArgb(45, 45, 45); status.Text = "Keep the USB connected. Setup is starting...";
                int lastPercent = -1;
                Action<int, string> report = delegate(int percent, string message)
                {
                    if (percent == lastPercent) return; lastPercent = percent;
                    BeginInvoke((Action)delegate { progress.Value = Math.Max(0, Math.Min(100, percent)); status.Text = message; });
                };
                string backup = await Task.Run(() => {
                    byte[] efi = Payload.Efi(), readme = Payload.Readme();
                    if (!plan.Erase) return FileInstall.Run(plan, probe, efi, readme, report);
                    using (RawUsb raw = new RawUsb(plan, probe))
                    {
                        Fat32Install.Run(raw, Fat32Layout.Create(plan.Disk.Size, plan.Disk.SectorSize), efi, readme, report);
                        raw.Complete();
                    }
                    return null;
                });
                ShowReady(backup);
            }
            catch (Exception e)
            {
                status.ForeColor = Color.Firebrick; status.Text = "USB setup did not finish.\n" + e.Message + "\nClose files on the USB, refresh the list, and try again.";
            }
            finally { SetBusy(false); UpdateChoice(); }
        }
        internal void SelectForPreview(UsbDisk disk)
        { devices.Items.Add(disk); devices.SelectedIndex = 0; }
        internal void ShowReady(string backup)
        {
            progress.Visible = true; progress.Value = 100;
            status.ForeColor = Color.FromArgb(30, 115, 62);
            status.Text = "USB ready.\n1. Restart your PC and open its boot menu.\n2. Select the USB's UEFI entry.\n3. Choose Size, Threads, and Runs, then press Enter." +
                (backup == null ? "" : "\nPrevious boot file saved as: " + Path.GetFileName(backup));
        }
    }

    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            AppContext.SetSwitch("Switch.System.IO.UseLegacyPathHandling", false);
            AppContext.SetSwitch("Switch.System.IO.BlockLongPaths", false);
            Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SetupForm(new WindowsUsbProbe(), true));
        }
    }
}
